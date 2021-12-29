#include "form_settings.h"
#include <cstdint>
#include <QToolTip>

#include "program_param.h"
#include <QScrollArea>

// todo: message view
FormSettings::FormSettings(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    createSettingsWidget();
    createButtons();

//    setFontPointSize(14);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(widgetSettings);
    scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea, Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

QGroupBox* FormSettings::groupLabelNum(QLabel* label, QDoubleSpinBox* num) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(num);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(num);
    return group;
}

QGroupBox* FormSettings::groupLabelNum(QLabel* label, QSpinBox* num, QComboBox* combo) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(num);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(num);
    group->layout()->addWidget(combo);
    return group;
}

QGroupBox* FormSettings::groupLabelCombo(QLabel* label, QComboBox* combo) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(combo);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(combo);
    return group;
}

void FormSettings::createSettingsWidget() {
    widgetSettings = new QWidget(this);
    gridSettings = new QGridLayout;

    labelTitle = new QLabel("<h3>" + tr("Application settings") + "</h3>");
    labelCNC = new QLabel("<h3>" + tr("CNC parameters") + "</h3>");
//    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelLanguage = new QLabel("Language: ");

    comboLanguage = new QComboBox;
    comboLanguage->addItem("English");
    comboLanguage->addItem("Русский");

    comboLanguage->setCurrentIndex(int(ProgramParam::lang));

    checkSwapXY = new QCheckBox(tr("Swap plot axes X, Y"));
    checkSwapXY->setCheckState(ProgramParam::swapXY ? Qt::Checked : Qt::Unchecked);

    checkReverseX = new QCheckBox(tr("Reverse plot axis X"));
    checkReverseX->setCheckState(ProgramParam::reverseX ? Qt::Checked : Qt::Unchecked);

    checkReverseY = new QCheckBox(tr("Reverse plot axis Y"));
    checkReverseY->setCheckState(ProgramParam::reverseY ? Qt::Checked : Qt::Unchecked);

    checkShowXY = new QCheckBox(tr("Show plot axes names"));
    checkShowXY->setCheckState(ProgramParam::showXY ? Qt::Checked : Qt::Unchecked);

    labelInputLevel = new QLabel(tr("Input Levels, bits") + ": ");
    numInputLevel = new QSpinBox;
    numInputLevel->setDisplayIntegerBase(16);
//    QFont font = spinKeyLevel->font();
//    font.setCapitalization(QFont::AllUppercase);
//    spinKeyLevel->setFont(font);
    numInputLevel->setPrefix("0x");
//    spinKeyLevel->setRange(INT32_MIN, INT32_MAX);
    numInputLevel->setRange(0, INT32_MAX);
    numInputLevel->setValue(int(CncParam::inputLevel));

    comboInputLevel = new QComboBox(this);
    comboInputLevel->addItem(tr(""));
    comboInputLevel->addItem(tr("Metal"));
    comboInputLevel->addItem(tr("Stone"));
    comboInputLevel->addItem(tr("Debug"));

    checkStepDirEnable = new QCheckBox(tr("Enable servomotors"));
    checkStepDirEnable->setCheckState(CncParam::sdEnable ? Qt::Checked : Qt::Unchecked);

    checkEncXY = new QCheckBox(tr("Enable XY linear encoders"));
    checkEncXY->setCheckState(CncParam::encXY ? Qt::Checked : Qt::Unchecked);

    checkReverseMotorX = new QCheckBox(tr("Reverse motor X"));
    checkReverseMotorX->setCheckState(CncParam::reverseX ? Qt::Checked : Qt::Unchecked);

    checkReverseMotorY = new QCheckBox(tr("Reverse motor Y"));
    checkReverseMotorY->setCheckState(CncParam::reverseY ? Qt::Checked : Qt::Unchecked);

    checkReverseMotorU = new QCheckBox(tr("Reverse motor U"));
    checkReverseMotorU->setCheckState(CncParam::reverseU ? Qt::Checked : Qt::Unchecked);

    checkReverseMotorV = new QCheckBox(tr("Reverse motor V"));
    checkReverseMotorV->setCheckState(CncParam::reverseV ? Qt::Checked : Qt::Unchecked);

    checkSwapMotorXY = new QCheckBox(tr("Swap motors X and Y"));
    checkSwapMotorXY->setCheckState(CncParam::swapXY ? Qt::Checked : Qt::Unchecked);

    checkSwapMotorUV = new QCheckBox(tr("Swap motors U and V"));
    checkSwapMotorUV->setCheckState(CncParam::swapUV ? Qt::Checked : Qt::Unchecked);

    checkReverseEncX = new QCheckBox(tr("Reverse Encoder X"));
    checkReverseEncX->setCheckState(CncParam::reverseEncX ? Qt::Checked : Qt::Unchecked);

    checkReverseEncY = new QCheckBox(tr("Reverse Encoder Y"));
    checkReverseEncY->setCheckState(CncParam::reverseEncY ? Qt::Checked : Qt::Unchecked);

    labelAcc = new QLabel(tr("Acceleration") + ":", this);
    fnumAcc = new QDoubleSpinBox(this);
    fnumAcc->setRange(1, 1000);
    fnumAcc->setSuffix(" " + tr("um/sec2")); // per 100 V
    fnumAcc->setDecimals(0);
    fnumAcc->setSingleStep(10);
    fnumAcc->setValue(CncParam::acc);
#ifdef STONE
    fnumAcc->setEnabled(false);
#endif

    labelDec = new QLabel(tr("Deceleration") + ":", this);
    fnumDec = new QDoubleSpinBox(this);
    fnumDec->setRange(1, 1000);
    fnumDec->setSuffix(" " + tr("um/sec2")); // per 100 V
    fnumDec->setDecimals(0);
    fnumDec->setSingleStep(10);
    fnumDec->setValue(CncParam::dec);
#ifdef STONE
    fnumDec->setEnabled(false);
#endif

    // Feedback
    groupFeedback = new QGroupBox(tr("Feedback enable"));
    groupFeedback->setCheckable(true);
    groupFeedback->setChecked(CncParam::fb_ena);

    labelHighThld = new QLabel(tr("High threshold"));
    labelLowThld = new QLabel(tr("Low threshold"));
    labelRbTimeout = new QLabel(tr("Rollback timeout"));
    labelRbAttempts = new QLabel(tr("Rollback attempts"));
    labelRbLength = new QLabel(tr("Rollback length"));
    labelRbSpeed = new QLabel(tr("Rollback speed"));

    int thld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );

    numHighThld = new QSpinBox;
    labelHighThld->setBuddy(numHighThld);
    numHighThld->setRange(0, thld_max);
    numHighThld->setValue(CncParam::high_thld);
    numHighThld->setSuffix(" " + tr("V"));

    numLowThld = new QSpinBox;
    labelLowThld->setBuddy(numLowThld);
    numLowThld->setRange(0, thld_max);
    numLowThld->setValue(CncParam::low_thld);
    numLowThld->setSuffix(" " + tr("V"));

    numRbTimeout = new QSpinBox;
    labelRbTimeout->setBuddy(numRbTimeout);
    numRbTimeout->setRange(10, 60);
    numRbTimeout->setSuffix(" " + tr("sec"));
    numRbTimeout->setValue(CncParam::rb_to);

    numRbAttempts = new QSpinBox;
    labelRbAttempts->setBuddy(numRbAttempts);
    numRbAttempts->setRange(1, 5);
    numRbAttempts->setValue(CncParam::rb_attempts);

    fnumRbLength = new QDoubleSpinBox;
    labelRbLength->setBuddy(fnumRbLength);
    fnumRbLength->setRange(0.100, 0.500);
    fnumRbLength->setSuffix(" " + tr("mm"));
    fnumRbLength->setDecimals(3);
    fnumRbLength->setSingleStep(0.01);
    fnumRbLength->setValue(CncParam::rb_len);

    fnumRbSpeed = new QDoubleSpinBox;
    labelRbSpeed->setBuddy(fnumRbSpeed);
    fnumRbSpeed->setRange(0.1, 2);
    fnumRbSpeed->setSuffix(" " + tr("mm/min"));
    fnumRbSpeed->setDecimals(1);
    fnumRbSpeed->setSingleStep(0.1);
    fnumRbSpeed->setValue(CncParam::rb_speed);

    QGridLayout* gridFeedback = new QGridLayout;
    gridFeedback->addWidget(labelHighThld, 0, 0);
    gridFeedback->addWidget(numHighThld, 0, 1);
    gridFeedback->addWidget(labelLowThld, 1, 0);
    gridFeedback->addWidget(numLowThld, 1, 1);
    gridFeedback->addWidget(labelRbTimeout, 2, 0);
    gridFeedback->addWidget(numRbTimeout, 2, 1);
    gridFeedback->addWidget(labelRbAttempts, 3, 0);
    gridFeedback->addWidget(numRbAttempts, 3, 1);
    gridFeedback->addWidget(labelRbLength, 4, 0);
    gridFeedback->addWidget(fnumRbLength, 4, 1);
    gridFeedback->addWidget(labelRbSpeed, 5, 0);
    gridFeedback->addWidget(fnumRbSpeed, 5, 1);

    groupFeedback->setLayout(gridFeedback);

    //    
    numStep = new QDoubleSpinBox;
    labelStep = new QLabel(tr("Calculation step") + ": ");
    labelStep->setBuddy(numStep);
    numStep->setValue(CncParam::step);
    numStep->setRange(0.001, 1.0);
    // todo: bind range with scale
    numStep->setSingleStep(0.001);
    numStep->setDecimals(3);
    numStep->setSuffix(" " + tr("mm"));

    labelX = new QLabel("X: ");
    labelY = new QLabel("Y: ");
    labelU = new QLabel("U: ");
    labelV = new QLabel("V: ");
    labelEncX = new QLabel("X: ");
    labelEncY = new QLabel("Y: ");

    labelPrecision = new QLabel(tr("Precision (steps/mm)"));
    labelMotor = new QLabel(tr("Motor"));
    labelEncoder = new QLabel(tr("Encoder"));

    //
    numScaleX = new QDoubleSpinBox;
    labelX->setBuddy(numScaleX);
    numScaleX->setRange(1, 1000000);
    numScaleX->setValue(CncParam::scaleX);
    numScaleX->setDecimals(0);

    numScaleY = new QDoubleSpinBox;
    labelY->setBuddy(numScaleY);
    numScaleY->setRange(1, 1000000);
    numScaleY->setValue(CncParam::scaleY);
    numScaleY->setDecimals(0);

    numScaleU = new QDoubleSpinBox;
    labelU->setBuddy(numScaleU);
    numScaleU->setRange(1, 1000000);
    numScaleU->setValue(CncParam::scaleU);
    numScaleU->setDecimals(0);

    numScaleV = new QDoubleSpinBox;
    labelV->setBuddy(numScaleV);
    numScaleV->setRange(1, 1000000);
    numScaleV->setValue(CncParam::scaleV);
    numScaleV->setDecimals(0);

    numScaleEncX = new QDoubleSpinBox;
    labelEncX->setBuddy(numScaleEncX);
    numScaleEncX->setRange(1, 1000);
    numScaleEncX->setValue(CncParam::scaleEncX);
    numScaleEncX->setDecimals(0);

    numScaleEncY = new QDoubleSpinBox;
    labelEncY->setBuddy(numScaleEncY);
    numScaleEncY->setRange(1, 1000);
    numScaleEncY->setValue(CncParam::scaleEncY);
    numScaleEncY->setDecimals(0);

    scaleNum = {numScaleX, numScaleY, numScaleU, numScaleV};
    encScaleNum = {numScaleEncX, numScaleEncY};

    setFontPointSize(16);

    gridSettings = new QGridLayout;

    gridSettings->addWidget(labelTitle, 0, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);
    gridSettings->addWidget(new QFrame, 1, 0, 1, 4);

    gridSettings->addWidget(groupLabelCombo(labelLanguage, comboLanguage), 2, 1, 1, 2);
    gridSettings->addWidget(checkSwapXY, 3, 1, 1, 4);
    gridSettings->addWidget(checkReverseX, 4, 1, 1, 4);
    gridSettings->addWidget(checkReverseY, 5, 1, 1, 4);
    gridSettings->addWidget(checkShowXY, 6, 1, 1, 4);

    gridSettings->addWidget(new QFrame, 7, 0, 1, 4);
    gridSettings->addWidget(labelCNC, 8, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);

    gridSettings->addWidget(groupLabelNum(labelInputLevel, numInputLevel, comboInputLevel), 9, 1, 1, 3);

    gridSettings->addWidget(groupLabelNum(labelStep, numStep), 10, 1, 1, 2);

#ifndef STONE
    gridSettings->addWidget(checkStepDirEnable, 11, 1, 1, 4);
    gridSettings->addWidget(checkEncXY, 12, 1, 1, 4);
#endif
    gridSettings->addWidget(checkReverseMotorX, 13, 1, 1, 4);
    gridSettings->addWidget(checkReverseMotorY, 14, 1, 1, 4);
#ifndef STONE
    gridSettings->addWidget(checkReverseMotorU, 15, 1, 1, 4);
    gridSettings->addWidget(checkReverseMotorV, 16, 1, 1, 4);
#endif

    gridSettings->addWidget(checkSwapMotorXY, 17, 1, 1, 4);
#ifndef STONE
    gridSettings->addWidget(checkSwapMotorUV, 18, 1, 1, 4);

    gridSettings->addWidget(checkReverseEncX, 19, 1, 1, 4);
    gridSettings->addWidget(checkReverseEncY, 20, 1, 1, 4);
#endif

#ifndef STONE
    gridSettings->addWidget(groupLabelNum(labelAcc, fnumAcc), 21, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelDec, fnumDec), 22, 1, 1, 2);

    gridSettings->addWidget(groupFeedback, 23, 1, 1, 2);
#endif

    gridSettings->addWidget(labelPrecision, 24, 1, 1, 4, Qt::AlignHCenter | Qt::AlignBottom);

    gridSettings->addWidget(labelMotor, 25, 0);
    gridSettings->addWidget(groupLabelNum(labelX, numScaleX), 25, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelY, numScaleY), 25, 3, 1, 2);

#ifndef STONE
    gridSettings->addWidget(groupLabelNum(labelU, numScaleU), 26, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelV, numScaleV), 26, 3, 1, 2);
#endif

    gridSettings->addWidget(labelEncoder, 27, 0);
    gridSettings->addWidget(groupLabelNum(labelEncX, numScaleEncX), 27, 1, 1, 2);
    gridSettings->addWidget(groupLabelNum(labelEncY, numScaleEncY), 27, 3, 1, 2);

//    gridSettings->addWidget(new QFrame, 0, 3, 8, 1);
//    gridSettings->addWidget(new QFrame, 8, 0, 1, 4);

//    gridSettings->setSizeConstraint(QLayout::SetFixedSize);
    QWidget* widgetInside = new QWidget(this);
    widgetInside->setLayout(gridSettings);

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(new QFrame, 0, 0);
    grid->addWidget(widgetInside, 0, 1);
    grid->addWidget(new QFrame, 0, 2);
    grid->addWidget(new QFrame, 1, 0, 1, 3);

    widgetSettings->setLayout(grid);

    //
    connect(comboLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int i) {
        switch (i) {
        case int(InterfaceLanguage::RUSSIAN):
            ProgramParam::saveInterfaceLanguage(InterfaceLanguage::RUSSIAN);
            break;
        default:
            ProgramParam::saveInterfaceLanguage(InterfaceLanguage::ENGLISH);
            break;
        }

        emit showWarning("Interface language will be changed after reboot");
    });

    connect(checkSwapXY, &QCheckBox::stateChanged, [&](int) {
        ProgramParam::saveSwapXY(checkSwapXY->isChecked());
    });

    connect(checkReverseX, &QCheckBox::stateChanged, [&](int) {
        ProgramParam::saveReverseX(checkReverseX->isChecked());
    });

    connect(checkReverseY, &QCheckBox::stateChanged, [&](int) {
        ProgramParam::saveReverseY(checkReverseY->isChecked());
    });

    connect(checkShowXY, &QCheckBox::stateChanged, [&](int) {
        ProgramParam::saveShowXY(checkShowXY->isChecked());
    });

    connect(comboInputLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int i) {
        numInputLevel->blockSignals(true);

        switch (i) {
        case 1: numInputLevel->setValue(CncParam::INPUT_LEVEL_METAL); break;
        case 2: numInputLevel->setValue(CncParam::INPUT_LEVEL_STONE); break;
        case 3: numInputLevel->setValue(CncParam::INPUT_LEVEL_DEBUG); break;
        }

        numInputLevel->blockSignals(false);
    });

    connect(numInputLevel, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int /*value*/) {
        comboInputLevel->setCurrentIndex(0);
    });

    connect(checkReverseMotorX, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseMotorY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseMotorU, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseMotorV, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkSwapMotorXY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkSwapMotorUV, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);    
    connect(checkReverseEncX, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);
    connect(checkReverseEncY, &QCheckBox::stateChanged, this, &FormSettings::onStateChangedMotor);

//    connect(numHighThld, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
//        if (value < numLowThld->value())
//            numLowThld->setValue(value);
//    });

//    connect(numLowThld, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
//        if (value > numHighThld->value())
//            numHighThld->setValue(value);
//    });

    connect(numScaleX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
    connect(numScaleY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormSettings::onNumScaleChanged);
}

void FormSettings::onNumScaleChanged(double) {
    double value = std::min(numScaleX->value(), numScaleY->value());
    double step = 1.0 / value;

    if (step < 0.001)
        step = 0.001;

    numStep->setMinimum(step);
}

void FormSettings::onStateChangedMotor(int /*state*/) {
    ProgramParam::saveMotorDir(
        checkReverseMotorX->isChecked(), checkReverseMotorY->isChecked(), checkReverseMotorU->isChecked(), checkReverseMotorV->isChecked(),
        checkSwapMotorXY->isChecked(), checkSwapMotorUV->isChecked(),
        checkReverseEncX->isChecked(), checkReverseEncY->isChecked()
    );
}

FormSettings::~FormSettings() {}

void FormSettings::setFontPointSize(int pointSize) {
    QFont font;

    for (QPushButton* b: buttons) {
        font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
    }

    font = labelX->font();
    font.setPointSize(pointSize + 2);

    labelTitle->setFont(font);
    labelCNC->setFont(font);
    labelLanguage->setFont(font);
    labelInputLevel->setFont(font);
    labelStep->setFont(font);
    labelAcc->setFont(font);
    labelDec->setFont(font);

    labelX->setFont(font);
    labelY->setFont(font);
    labelU->setFont(font);
    labelV->setFont(font);
    labelEncX->setFont(font);
    labelEncY->setFont(font);

    labelPrecision->setFont(font);
    labelMotor->setFont(font);
    labelEncoder->setFont(font);

    //
    font = comboLanguage->font();
    font.setPointSize(pointSize + 2);

    comboLanguage->setFont(font);
    comboInputLevel->setFont(font);

    //
    font = numStep->font();
    font.setPointSize(pointSize + 2);

    numInputLevel->setFont(font);
    numStep->setFont(font);
    fnumAcc->setFont(font);
    fnumDec->setFont(font);

    numScaleX->setFont(font);
    numScaleY->setFont(font);
    numScaleU->setFont(font);
    numScaleV->setFont(font);
    numScaleEncX->setFont(font);
    numScaleEncY->setFont(font);

    font = checkSwapXY->font();
    font.setPointSize(pointSize + 2);
    checkSwapXY->setFont(font);
    checkReverseX->setFont(font);
    checkReverseY->setFont(font);
    checkShowXY->setFont(font);
    checkStepDirEnable->setFont(font);
    checkEncXY->setFont(font);
    checkReverseMotorX->setFont(font);
    checkReverseMotorY->setFont(font);
    checkReverseMotorU->setFont(font);
    checkReverseMotorV->setFont(font);
    checkSwapMotorXY->setFont(font);
    checkSwapMotorUV->setFont(font);
    checkReverseEncX->setFont(font);
    checkReverseEncY->setFont(font);

    groupFeedback->setFont(font);
    labelHighThld->setFont(font);
    labelLowThld->setFont(font);
    numHighThld->setFont(font);
    numLowThld->setFont(font);
}

void FormSettings::createButtons() {
    btnHome = new QPushButton(tr("Back"));
    btnHome->setStatusTip(tr("Back to the home panel"));

    btnRead = new QPushButton(tr("Read"));
    btnRead->setStatusTip(tr("Read settings from CNC"));

    btnWrite = new QPushButton(tr("Write"));
    btnWrite->setStatusTip(tr("Write settings"));

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setStatusTip(tr("Default settings"));

    btn4 = new QPushButton();
    btn4->setText(tr(""));
    btn4->setEnabled(false);

    btn5 = new QPushButton();
    btn5->setText(tr(""));
    btn5->setEnabled(false);

    btn6 = new QPushButton();
    btn6->setText(tr(""));
    btn6->setEnabled(false);

    btn7 = new QPushButton();
    btn7->setText(tr(""));
    btn7->setEnabled(false);

    btn8 = new QPushButton();
    btn8->setText(tr(""));
    btn8->setEnabled(false);

    btn9 = new QPushButton();
    btn9->setText(tr(""));
    btn9->setEnabled(false);

    btn10 = new QPushButton();
    btn10->setText(tr(""));
    btn10->setEnabled(false);

    btn11 = new QPushButton();
    btn11->setText(tr(""));
    btn11->setEnabled(false);

    btn12 = new QPushButton();
    btn12->setText(tr(""));
    btn12->setEnabled(false);

    btnHelp = new QPushButton();
    btnHelp->setText(tr("Help"));

    buttons = {btnHome, btnRead, btnWrite, btnDefault, btn4, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btn12, btnHelp};

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnRead, 0, 1);
    gridButtons->addWidget(btnWrite, 0, 2);
    gridButtons->addWidget(btnDefault, 0, 3);
    gridButtons->addWidget(btn4, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnHome, &QPushButton::clicked, this, [&]() {
        emit showInfo(QString());
        emit homePageClicked();
    });

    connect(btnRead, &QPushButton::clicked, this, [&]() {
        uint16_t input_lvl;
        bool sdEna, revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY;
        bool fbEna;
        uint32_t rbAttempts;
        double low_thld, high_thld, rbTimeout, rbLength, rbSpeed, acc, dec;

        float step;
        bool encXY;
        float scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY;

        bool OK = par.cnc.readSettings(
                    input_lvl, sdEna, sdEna, revX, revY, revU, revV,
                    swapXY, swapUV,
                    revEncX, revEncY,
                    acc, dec
        );

        if (OK) {
            ProgramParam::saveInputLevel(input_lvl);
            numInputLevel->setValue(input_lvl);
            comboInputLevel->setCurrentIndex(0);

            ProgramParam::saveStepDir(sdEna);
            checkStepDirEnable->setCheckState(sdEna ? Qt::Checked : Qt::Unchecked);

            ProgramParam::saveMotorDir(revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY);            
            checkReverseMotorX->setCheckState(revX ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorY->setCheckState(revY ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorU->setCheckState(revU ? Qt::Checked : Qt::Unchecked);
            checkReverseMotorV->setCheckState(revV ? Qt::Checked : Qt::Unchecked);
            checkSwapMotorXY->setCheckState(swapXY ? Qt::Checked : Qt::Unchecked);
            checkSwapMotorUV->setCheckState(swapUV ? Qt::Checked : Qt::Unchecked);
            checkReverseEncX->setCheckState(revEncX ? Qt::Checked : Qt::Unchecked);
            checkReverseEncY->setCheckState(revEncY ? Qt::Checked : Qt::Unchecked);

            ProgramParam::saveAcceleration(acc, dec);
            fnumAcc->setValue(acc);
            fnumDec->setValue(dec);
        }
        else
            qDebug("Read Input Levels ERROR!\n");

        if (OK)
            OK = par.cnc.readFeedback(fbEna, low_thld, high_thld, rbTimeout, rbAttempts, rbLength, rbSpeed);

        if (OK) {
            ProgramParam::saveFeedbackParam(fbEna, low_thld, high_thld, rbTimeout, rbAttempts, rbLength, rbSpeed);

            numLowThld->blockSignals(true);
            numHighThld->blockSignals(true);

            // todo: save param
            groupFeedback->setChecked(fbEna);
            numLowThld->setValue(static_cast<int>(round(low_thld)));
            numHighThld->setValue(static_cast<int>(round(high_thld)));
            numRbTimeout->setValue(rbTimeout);
            numRbAttempts->setValue(rbAttempts);
            fnumRbLength->setValue(rbLength);
            fnumRbSpeed->setValue(rbSpeed);

            numLowThld->blockSignals(false);
            numHighThld->blockSignals(false);
            emit showInfo("Read OK");
        }
        else {
            groupFeedback->setChecked(false);
            emit showError("Read ERROR!");
        }

        if (OK)
            OK = par.cnc.readStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);

        if (OK) {
            ProgramParam::saveStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);
            numStep->setValue(step);
            numScaleX->setValue(scaleX);
            numScaleY->setValue(scaleY);
            numScaleU->setValue(scaleU);
            numScaleV->setValue(scaleV);
            numScaleEncX->setValue(scaleEncX);
            numScaleEncY->setValue(scaleEncY);
            checkEncXY->setCheckState(encXY ? Qt::Checked : Qt::Unchecked);
        }
    });

    connect(btnWrite, &QPushButton::clicked, this, [&]() {
        uint16_t input_lvl = static_cast<uint16_t>(numInputLevel->value() & 0xFFFF);
        bool sd_oe = checkStepDirEnable->isChecked();
        bool encXY = checkEncXY->isChecked();
        bool revX = checkReverseMotorX->isChecked();
        bool revY = checkReverseMotorY->isChecked();
        bool revU = checkReverseMotorU->isChecked();
        bool revV = checkReverseMotorV->isChecked();
        bool swapXY = checkSwapMotorXY->isChecked();
        bool swapUV = checkSwapMotorUV->isChecked();
        bool revEncX = checkReverseEncX->isChecked();
        bool revEncY = checkReverseEncY->isChecked();        
        double acc = fnumAcc->value();
        double dec = fnumDec->value();

        float step = numStep->value();
        float scaleX = numScaleX->value();
        float scaleY = numScaleY->value();
        float scaleU = numScaleU->value();
        float scaleV = numScaleV->value();
        float scaleEncX = numScaleEncX->value();
        float scaleEncY = numScaleEncY->value();

        ProgramParam::saveInputLevel(input_lvl);
        ProgramParam::saveMotorDir(revX, revY, revU, revV, swapXY, swapUV, revEncX, revEncY);
        ProgramParam::saveAcceleration(acc, dec);
        ProgramParam::saveStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);

        bool OK =   par.cnc.writeSettings(
                        input_lvl,
                        sd_oe, sd_oe,
                        revX, revY, revU, revV,
                        swapXY, swapUV,
                        revEncX, revEncY,
                        acc, dec
                    );


        if (numHighThld->value() < numLowThld->value())
            numHighThld->setValue( numLowThld->value() );

        ProgramParam::saveFeedbackParam(
                    groupFeedback->isChecked(),
                    numLowThld->value(),
                    numHighThld->value(),
                    numRbTimeout->value(),
                    numRbAttempts->value(),
                    fnumRbLength->value(),
                    fnumRbSpeed->value()
                );

        if (OK)
            OK &=   par.cnc.writeFeedback(
                    groupFeedback->isChecked(),
                    numLowThld->value(),
                    numHighThld->value(),
                    numRbTimeout->value(),
                    numRbAttempts->value(),
                    fnumRbLength->value(),
                    fnumRbSpeed->value()
                );

        if (OK)
            OK &= par.cnc.writeStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);

#if defined(STONE)
        if (OK)
            OK &= par.cnc.writeSemaphoreCncEnable(true);
#else
        if (OK)
            OK &= par.cnc.writeCncEnable(true);
#endif

        if (OK) {
            emit showInfo("Write OK");
            btnRead->click();
        }
        else
            emit showError("Write ERROR!");
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}
