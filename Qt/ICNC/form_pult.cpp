#include "form_pult.h"
#include <QDebug>
#include <QTimer>

using namespace std;

FormPult::FormPult(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), cncReaderEna(false), adcEnable(false), cutStateAbortReq(false), wireSpeedMode(WireSpeed::Mode::MMM)
{
    pultWidget = new PultWidget;
    createButtonLayout();

    buttons = {btnHome, btnDrum, btnPump, btnBreak, btnVoltage, btnWidthDec, btnWidthInc, btnRatioInc, btnRatioDec,\
            btnVoltageDec, btnVoltageInc, btnCurrentDec, btnCurrentInc, btnDrumVelDec, btnDrumVelInc, btnHelp};

//    setFontPointSize(14);

    QLabel* labelTitle = new QLabel(R"(<h1>)" + tr("Pult") + R"(</h1>)");

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(labelTitle, 0, Qt::AlignCenter);
    mainLayout->addWidget(pultWidget);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    connect(pultWidget->btnGo, &QPushButton::clicked, this, &FormPult::on_btnMove_clicked);
    connect(pultWidget->btnSet, &QPushButton::clicked, this, &FormPult::on_btnSet_clicked);

    connect(pultWidget->btnCancel, &QPushButton::clicked, this, &FormPult::on_btnCancel_clicked);

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );
}

FormPult::~FormPult() {
    stopCncReader();
    stopAdc();
}

void FormPult::setFontPointSize(int pointSize) {
    pultWidget->setFontPointSize(pointSize);

    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
    }

    for (QSpinBox* const numBox: nums) {
        QFont font = numBox->font();
        font.setPointSize(pointSize);
        numBox->setFont(font);
    }

    for (QLabel* const label: labels) {
        QFont font = label->font();
        font.setPointSize(pointSize);
        label->setFont(font);
    }
}

void FormPult::addButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to Home panel"));

    btnHelp = new QPushButton(tr("Help"));

    btnBreak = new QPushButton(tr("Break"));
    btnBreak->setCheckable(true);
    btnBreak->setStatusTip(tr("Wire break control on/off"));

    btnPump = new QPushButton(tr("Pump"));
    btnPump->setCheckable(true);
    btnPump->setStatusTip(tr("Pump on/off"));

    btnDrum = new QPushButton(tr("Drum"));
    btnDrum->setCheckable(true);
    btnDrum->setStatusTip(tr("Wire drum on/off"));

    btnDrumVelDec = new QPushButton(tr("DEC"));
    btnDrumVelDec->setStatusTip(tr("Drum velocity decrement"));

    btnDrumVelInc = new QPushButton(tr("INC"));
    btnDrumVelInc->setStatusTip(tr("Drum velocity increment"));

    btnVoltage = new QPushButton;
#ifndef STONE
    btnVoltage->setText(tr("Voltage"));
    btnVoltage->setCheckable(true);
    btnVoltage->setStatusTip(tr("Enable high voltage if drum and pump enabled"));
#else
    btnVoltage->setEnabled(false);
#endif

    btnVoltageDec = new QPushButton;
#ifndef STONE
    btnVoltageDec->setText(tr("DEC"));
#else
    btnVoltageDec->setEnabled(false);
#endif

    btnVoltageInc = new QPushButton;
#ifndef STONE
    btnVoltageInc->setText(tr("INC"));
#else
    btnVoltageInc->setEnabled(false);
#endif

    btnCurrentDec = new QPushButton;
#ifndef STONE
    btnCurrentDec->setText(tr("DEC"));
#else
    btnCurrentDec->setEnabled(false);
#endif

    btnCurrentInc = new QPushButton;
#ifndef STONE
    btnCurrentInc->setText(tr("INC"));
#else
    btnCurrentInc->setEnabled(false);
#endif

    btnWidthDec = new QPushButton;
#ifndef STONE
    btnWidthDec->setText(tr("DEC"));
#else
    btnWidthDec->setEnabled(false);
#endif

    btnWidthInc = new QPushButton;
#ifndef STONE
    btnWidthInc->setText(tr("INC"));
#else
    btnWidthInc->setEnabled(false);
#endif

    btnRatioDec = new QPushButton;
#ifndef STONE
    btnRatioDec->setText(tr("DEC"));
#else
    btnRatioDec->setEnabled(false);
#endif

    btnRatioInc = new QPushButton;
#ifndef STONE
    btnRatioInc->setText(tr("INC"));
#else
    btnRatioInc->setEnabled(false);
#endif

    buttons = {btnHome, btnHelp, btnBreak, btnPump, btnDrum, btnDrumVelDec, btnDrumVelInc,\
               btnVoltage, btnVoltageDec, btnVoltageInc, btnCurrentDec, btnCurrentInc, btnWidthDec, btnWidthInc, btnRatioInc, btnRatioDec};

#ifdef STONE
    gridButtons->addWidget(btnHome, 1, 0);
    gridButtons->addWidget(btnBreak, 1, 1);
    gridButtons->addWidget(btnPump, 1, 2);
    gridButtons->addWidget(btnDrum, 1, 3);
    gridButtons->addWidget(btnDrumVelDec, 1, 4);
    gridButtons->addWidget(btnDrumVelInc, 1, 5);
    gridButtons->addWidget(btnVoltage, 1, 6);
    gridButtons->addWidget(btnVoltageDec, 1, 7);
    gridButtons->addWidget(btnVoltageInc, 1, 8);
    gridButtons->addWidget(btnCurrentDec, 1, 9);
    gridButtons->addWidget(btnCurrentInc, 1, 10);
    gridButtons->addWidget(btnWidthDec, 1, 11);
    gridButtons->addWidget(btnWidthInc, 1, 12);
    gridButtons->addWidget(btnHelp, 1, 13);
#else
    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnHelp, 0, 1);

    gridButtons->addWidget(btnBreak, 1, 0);
    gridButtons->addWidget(btnPump, 1, 1);
    gridButtons->addWidget(btnDrum, 1, 2);
    gridButtons->addWidget(btnDrumVelDec, 1, 3);
    gridButtons->addWidget(btnDrumVelInc, 1, 4);
    gridButtons->addWidget(btnVoltage, 1, 5);
    gridButtons->addWidget(btnVoltageDec, 1, 6);
    gridButtons->addWidget(btnVoltageInc, 1, 7);
    gridButtons->addWidget(btnCurrentDec, 1, 8);
    gridButtons->addWidget(btnCurrentInc, 1, 9);
    gridButtons->addWidget(btnWidthDec, 1, 10);
    gridButtons->addWidget(btnWidthInc, 1, 11);
    gridButtons->addWidget(btnRatioDec, 1, 12);
    gridButtons->addWidget(btnRatioInc, 1, 13);
#endif

    connect(btnHome, &QPushButton::clicked, this, &FormPult::on_btnHome_clicked);
    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });

    connect(btnDrum, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writeDrumEnable( btnDrum->isChecked() );
        }
        catch (...) {}
    });

    connect(btnPump, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writePumpEnable( btnPump->isChecked() );
        }
        catch (...) {}
    });

    connect(btnBreak, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writeWireEnable( btnBreak->isChecked() );
        }
        catch (...) {}
    });

    connect(btnVoltage, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writeVoltageEnable( btnVoltage->isChecked() );
        }
        catch (...) {}
    });

    connect(pultWidget->btnHold, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writeHoldEnable( pultWidget->btnHold->isChecked() );
        }
        catch (...) {}
    });

    connect(btnDrumVelInc, &QPushButton::clicked, [&]() { numDrumVel->stepUp(); });
    connect(btnDrumVelDec, &QPushButton::clicked, [&]() { numDrumVel->stepDown(); });
    connect(btnWidthInc, &QPushButton::clicked, [&]() { numWidth->stepUp(); });
    connect(btnWidthDec, &QPushButton::clicked, [&]() { numWidth->stepDown(); });
    connect(btnRatioInc, &QPushButton::clicked, [&]() { numRatio->stepUp(); });
    connect(btnRatioDec, &QPushButton::clicked, [&]() { numRatio->stepDown(); });
    connect(btnVoltageInc, &QPushButton::clicked, [&]() { numVoltage->stepUp(); });
    connect(btnVoltageDec, &QPushButton::clicked, [&]() { numVoltage->stepDown(); });
    connect(btnCurrentInc, &QPushButton::clicked, [&]() { numCurrent->stepUp(); });
    connect(btnCurrentDec, &QPushButton::clicked, [&]() { numCurrent->stepDown(); });
}

void FormPult::createButtonLayout() {
    numDrumVel = new QSpinBox;
    numDrumVel->setRange(1, 7);
    numWidth = new QSpinBox;
    numWidth->setRange(2, 99);
    numRatio = new QSpinBox;
    numRatio->setRange(2, 32);
    numVoltage = new QSpinBox;
    numVoltage->setRange(0, 1);
    numCurrent = new QSpinBox;
    numCurrent->setRange(0, 11);

    nums = {numDrumVel, numWidth, numRatio, numVoltage, numCurrent};

    labelDrum = new QLabel(tr("Drum") + ": ");
    labelDrum->setBuddy(numDrumVel);
    labelWidth = new QLabel(tr("Width") + ": ");
    labelWidth->setBuddy(numWidth);
    labelRatio = new QLabel(tr("Ratio") + ": ");
    labelRatio->setBuddy(numRatio);
    labelVoltage = new QLabel(tr("Voltage") + ": ");
    labelVoltage->setBuddy(numVoltage);
    labelCurrent = new QLabel(tr("Current") + ": ");
    labelCurrent->setBuddy(numCurrent);

    labels = {labelDrum, labelWidth, labelRatio, labelVoltage, labelCurrent};

    groupDrum = new QGroupBox;
    groupWidth = new QGroupBox;
    groupRatio = new QGroupBox;
    groupVoltage = new QGroupBox;
    groupCurrent = new QGroupBox;

    groupDrum->setLayout(new QHBoxLayout);
    groupDrum->layout()->addWidget(labelDrum);
    groupDrum->layout()->addWidget(numDrumVel);

    groupWidth->setLayout(new QHBoxLayout);
    groupWidth->layout()->addWidget(labelWidth);
    groupWidth->layout()->addWidget(numWidth);

    groupRatio->setLayout(new QHBoxLayout);
    groupRatio->layout()->addWidget(labelRatio);
    groupRatio->layout()->addWidget(numRatio);

    groupVoltage->setLayout(new QHBoxLayout);
    groupVoltage->layout()->addWidget(labelVoltage);
    groupVoltage->layout()->addWidget(numVoltage);

    groupCurrent->setLayout(new QHBoxLayout);
    groupCurrent->layout()->addWidget(labelCurrent);
    groupCurrent->layout()->addWidget(numCurrent);

    gridButtons = new QGridLayout;

#ifdef STONE
    gridButtons->addWidget(new QFrame, 0, 0, 1, 4);
    gridButtons->addWidget(groupDrum, 0, 4, 1, 2);
    gridButtons->addWidget(new QFrame, 0, 6, 1, 8);
#else
    gridButtons->addWidget(new QFrame, 0, 2);
    gridButtons->addWidget(groupDrum, 0, 3, 1, 2);
    gridButtons->addWidget(new QFrame, 0, 5);
    gridButtons->addWidget(groupVoltage, 0, 6, 1, 2);
    gridButtons->addWidget(groupCurrent, 0, 8, 1, 2);
    gridButtons->addWidget(groupWidth, 0, 10, 1, 2);
    gridButtons->addWidget(groupRatio, 0, 12, 1, 2);
#endif

    connect(numDrumVel, QOverload<int>::of(&QSpinBox::valueChanged), [=](int x) { par.cnc.writeDrumVel(static_cast<unsigned>(x)); });
    connect(numWidth, QOverload<int>::of(&QSpinBox::valueChanged), [=](int x) { par.cnc.writePulseWidth(static_cast<unsigned>(x)); });
    connect(numRatio, QOverload<int>::of(&QSpinBox::valueChanged), [=](int x) { par.cnc.writePulseRatio(static_cast<unsigned>(x)); });
    connect(numVoltage, QOverload<int>::of(&QSpinBox::valueChanged), [=](int x) { par.cnc.writeVolgateLevel(static_cast<unsigned>(x)); });
    connect(numCurrent, QOverload<int>::of(&QSpinBox::valueChanged), [=](int x) { par.cnc.writeCurrentIndex(static_cast<unsigned>(x)); });

    //
    WireSpeed speed;
    pultWidget->numSpeed->setRange(speed.min(), speed.max());
    pultWidget->numSpeed->setValue(speed.get());

    connect(pultWidget->numSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double x) {
        WireSpeed speed(x, wireSpeedMode);
        par.cnc.writeSpeed(speed);
    });

    connect(pultWidget->speedMMM, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::MMM) {
            WireSpeed speed(pultWidget->numSpeed->value(), wireSpeedMode);
            wireSpeedMode = WireSpeed::Mode::MMM;
            speed.changeMode(wireSpeedMode);
            pultWidget->numSpeed->setRange(speed.min(), speed.max());
            pultWidget->numSpeed->setValue(speed.get());
            pultWidget->numSpeed->setSingleStep(0.1);
            pultWidget->numSpeed->setDecimals(2);
        }
    });
    connect(pultWidget->speedUMS, &QRadioButton::clicked, this, [&]() {
        if (wireSpeedMode != WireSpeed::Mode::UMS) {
            WireSpeed speed(pultWidget->numSpeed->value(), wireSpeedMode);
            wireSpeedMode = WireSpeed::Mode::UMS;
            speed.changeMode(wireSpeedMode);
            pultWidget->numSpeed->setRange(speed.min(), speed.max());
            pultWidget->numSpeed->setValue(speed.get());
            pultWidget->numSpeed->setSingleStep(1);
            pultWidget->numSpeed->setDecimals(1);
        }
    });

    addButtons();
}

void FormPult::init() {
    _init();

    if (!cncReaderEna)
        startCncReader();

#ifndef STONE
    startAdc();
#endif
}

void FormPult::_init() {
    initButtons();

//    if (!par.cnc.isOpen())
//        pultWidget->txtMsg->insertPlainText("Error: No CNC connection\n");
}

void FormPult::on_btnHome_clicked() {
    stopCncReader();
    stopAdc();
    emit homePageClicked();
}

void FormPult::on_btnMove_clicked() {
    int32_t nx = pultWidget->getMoveN(0);
    int32_t ny = pultWidget->getMoveN(1);
    int32_t nu = pultWidget->getMoveN(2);
    int32_t nv = pultWidget->getMoveN(3);
    par.cnc.directMoveOn(nx, pultWidget->scale[0], ny, pultWidget->scale[1], nu, pultWidget->scale[2], nv, pultWidget->scale[3], pultWidget->speed());
}

void FormPult::on_btnSet_clicked() {
    int32_t nx = pultWidget->getSetN(0);
    int32_t ny = pultWidget->getSetN(1);
    int32_t nu = pultWidget->getSetN(2);
    int32_t nv = pultWidget->getSetN(3);
    int32_t enc_x = pultWidget->getSetEncN(0);
    int32_t enc_y = pultWidget->getSetEncN(1);
    par.cnc.directSetPos(nx, ny, nu, nv, enc_x, enc_y);
}

void FormPult::on_btnCancel_clicked() {
    par.cnc.cancelReq();
}

void FormPult::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";
        cncReaderEna = true;
        readCutState();
    }
}

void FormPult::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    cncReaderEna = false;
}

void FormPult::readCutState() {
    if (cncReaderEna) {
//        qDebug()<<"CNC reader: Reading...";

        try {
            CncContext ctx = par.cnc.readCncContext();
//            qDebug() << "CNC reader: " + QString(cut_state.toString().c_str());

            pultWidget->setMotorView(0, ctx.x());
            pultWidget->setMotorView(1, ctx.y());
            pultWidget->setMotorView(2, ctx.u());
            pultWidget->setMotorView(3, ctx.v());
            pultWidget->setEncoderView(0, ctx.encoderX());
            pultWidget->setEncoderView(1, ctx.encoderY());

            pultWidget->setLimitSwitches(ctx.limitSwitches());

            btnDrum->blockSignals(true);
            btnPump->blockSignals(true);
            btnBreak->blockSignals(true);
            btnVoltage->blockSignals(true);
            numDrumVel->blockSignals(true);
            numWidth->blockSignals(true);
            numRatio->blockSignals(true);
            numVoltage->blockSignals(true);
            numCurrent->blockSignals(true);

            pultWidget->btnHold->blockSignals(true);
            pultWidget->numSpeed->blockSignals(true);

            btnDrum->setChecked(ctx.isDrumEnable());
            btnPump->setChecked(ctx.pumpEnabled());
            btnBreak->setChecked(ctx.wireControlEnabled());
            btnVoltage->setChecked(ctx.voltageEnabled());

            pultWidget->btnHold->setChecked(ctx.hold());

            if (ctx.drumVelocity() < numDrumVel->minimum()) {
                numDrumVel->setValue(numDrumVel->minimum());
//                qDebug("Error: MIN CNC Drum Vel: %x", static_cast<unsigned>(ctx.drumVelocity()));
            }
            else if (ctx.drumVelocity() > numDrumVel->maximum()) {
                numDrumVel->setValue(numDrumVel->maximum());
                qDebug("Error: MAX CNC Drum Vel: %x", static_cast<unsigned>(ctx.drumVelocity()));
            }
            else
                numDrumVel->setValue(ctx.drumVelocity());

            if (ctx.pulseWidth() < numWidth->minimum()) {
                numWidth->setValue(numWidth->minimum());
                qDebug("Error: MIN CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else if (ctx.pulseWidth() > numWidth->maximum()) {
                numWidth->setValue(numWidth->maximum());
                qDebug("Error: MAX CNC Pulse Width: %x", static_cast<unsigned>(ctx.pulseWidth()));
            }
            else
                numWidth->setValue(ctx.pulseWidth());

            if (ctx.pulseRatio() < numRatio->minimum()) {
                numRatio->setValue(numRatio->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else if (ctx.pulseRatio() > numRatio->maximum()) {
                numRatio->setValue(numRatio->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.pulseRatio()));
            }
            else
                numRatio->setValue(ctx.pulseRatio());

            if (ctx.voltageLevel() < numVoltage->minimum()) {
                numVoltage->setValue(numVoltage->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.voltageLevel()));
            }
            else if (ctx.voltageLevel() > numVoltage->maximum()) {
                numVoltage->setValue(numVoltage->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.voltageLevel()));
            }
            else
                numVoltage->setValue(ctx.voltageLevel());

            if (ctx.currentIndex() < numCurrent->minimum()) {
                numCurrent->setValue(numCurrent->minimum());
                qDebug("Error: MIN CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else if (ctx.currentIndex() > numCurrent->maximum()) {
                numCurrent->setValue(numCurrent->maximum());
                qDebug("Error: MAX CNC Pulse Ratio: %x", static_cast<unsigned>(ctx.currentIndex()));
            }
            else
                numCurrent->setValue(ctx.currentIndex());

            WireSpeed speed( ctx.speed() );
            speed.changeMode(wireSpeedMode);

            if (speed.get() < speed.min()) {
                pultWidget->numSpeed->setValue(speed.min());
                qDebug("Error: MIN CNC Speed: %f", static_cast<double>(ctx.speed()));
            }
            else if (speed.get() > speed.max()) {
                pultWidget->numSpeed->setValue(speed.max());
                qDebug("Error: MAX CNC Speed: %f", static_cast<double>(ctx.speed()));
            }
            else
                pultWidget->numSpeed->setValue(speed.get());

            btnDrum->blockSignals(false);
            btnPump->blockSignals(false);
            btnBreak->blockSignals(false);
            btnVoltage->blockSignals(false);
            numDrumVel->blockSignals(false);
            numWidth->blockSignals(false);
            numRatio->blockSignals(false);
            numVoltage->blockSignals(false);
            numCurrent->blockSignals(false);

            pultWidget->btnHold->blockSignals(false);
            pultWidget->numSpeed->blockSignals(false);

//            if (par.appState.isWork() && cut_state.isWork()) {
//                onFrameChanged(cut_state.frame_num,
//                               fpoint_t(cut_state.pos.x / X_SCALE, cut_state.pos.y / Y_SCALE),
//                               fpoint_t(cut_state.pos.u / U_SCALE, cut_state.pos.v /  V_SCALE)
//                               );
//            }

            if (ctx.isWork()) {
                pultWidget->btnGo->setEnabled(false);
                pultWidget->btnSet->setEnabled(false);
                pultWidget->btnApply->setEnabled(false);
                pultWidget->btnCancel->setEnabled(true);
                btnHome->setEnabled(false);
            }
            else if (ctx.isError()) {
                par.cnc.cancelReq();
                initButtons();
            }
            else
                initButtons();
        }
        catch (...) {
            initButtons();
        }

        QTimer::singleShot(POLLING_TIME, this, &FormPult::readCutState);
    }
}

void FormPult::initButtons() {
    pultWidget->btnGo->setEnabled(true);
    pultWidget->btnSet->setEnabled(true);
    pultWidget->btnApply->setEnabled(true);
    pultWidget->btnCancel->setEnabled(false);
    btnHome->setEnabled(true);
}

void FormPult::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";
        adcEnable = true;
        readAdc();
    }
    else
        adcEnable = false;
}

void FormPult::stopAdc() { // at to home
    qDebug()<<"ADC: Stop";
    adcEnable = false;
}

void FormPult::readAdc() {
    if (adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        pultWidget->setAdc(adc);
        QTimer::singleShot(ADC_POLLING_TIME, this, &FormPult::readAdc);
    }
}
