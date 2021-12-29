#include "form_passes_stone.h"
#include <QSpacerItem>
#include "spinbox_delegate.h"
#include <QHeaderView>
#include <QStandardItemModel>
#include "mode_table_model.h"
#include <QLineEdit>
#include "cnc_types.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace std;

FormPassesStone::FormPassesStone(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par), m_pass_num(1), m_row(0) {
    QLabel* labelTitle = new QLabel(R"(<h1>)" + tr("Contour Adjustments") + R"(</h1>)");
    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    createPasses();
//    createModeView();
    createButtons();

    buttons = {btnBack, btnOpen, btnSave, btnSaveAs, btnDefault, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btn12, btnHelp/*,
               btnNewMode, btnDeleteMode, btnOpenModes, btnSaveModes, btnSaveAsModes, btnDefaultModes*/
              };
    labels = {/*labelTimes,*/ labelAuxOffset, labelOverlap, labelTab, /*labelTabOffset, labelTabMode, labelCutMode, labelPumpDelay,*/ labelSpeed/*, tableTitle*/};
    radio = {radioLeftAux, radioRightAux/*, onePassTab, multiPassTab*/, radioTabNone, radioTabLength, radioTabLast, radioLeftSide, radioRightSide};
//    combo = {comboTimes, comboCutMode, comboTabMode};
    checks = {checkAuxOffset, checkOffset, checkTabPause/*, checkPumpPause*/, checkSegmentPause};
    nums = {inputAuxOffset, inputOverlap, inputTab/*, inputTabOffset*/, inputSpeed};

//    setFontPointSize(14);

    gridPassMode = new QGridLayout;

    gridPassMode->addWidget(labelTitle, 0, 0, Qt::AlignHCenter);
    gridPassMode->addLayout(gridPasses, 1, 0, Qt::AlignVCenter | Qt::AlignHCenter);

////    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Preferred), 1, 1);
//    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 1);
////    gridPassMode->addLayout(gridMode, 1, 2, Qt::AlignVCenter | Qt::AlignLeft);

//    gridPassMode->setColumnStretch(0, 1);
//    gridPassMode->setColumnStretch(1, 0);
//    gridPassMode->setColumnStretch(2, 1);

    gridPassMode->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridPassMode);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

void FormPassesStone::createPasses() {
    // Scaling offset
    labelAuxOffset = new QLabel(tr("Additional Offset"));
    inputAuxOffset = new QDoubleSpinBox;
    labelAuxOffset->setBuddy(inputAuxOffset);
    inputAuxOffset->setSuffix(" " + tr("mm"));
    inputAuxOffset->setDecimals(3);
    inputAuxOffset->setSingleStep(1);
    inputAuxOffset->setMinimum(0);
    inputAuxOffset->setMaximum(1000);

    checkAuxOffset = new QCheckBox;

    radioLeftAux = new QRadioButton(tr("Left Offset"));
    radioRightAux = new QRadioButton(tr("Right Offset"));

    groupAuxOffset = new QGroupBox;
    {
        QGridLayout *grid = new QGridLayout;

        grid->addWidget(checkAuxOffset, 0, 0);
        grid->addWidget(inputAuxOffset, 0, 1, 1, 2);
        grid->addWidget(radioLeftAux, 1, 1);
        grid->addWidget(radioRightAux, 1, 2);
        groupAuxOffset->setLayout(grid);
    }
    groupAuxOffset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Cutting offset
    labelPasses = new QLabel*[par.cut.MAX_TIMES];
    inputOffsets = new QDoubleSpinBox*[par.cut.MAX_TIMES];

    labelPasses[0] = new QLabel(tr("Wire Offset"));
    inputOffsets[0] = new QDoubleSpinBox;
    labelPasses[0]->setBuddy(inputOffsets[0]);
    inputOffsets[0]->setSuffix(" " + tr("mm"));
    inputOffsets[0]->setDecimals(3);
    inputOffsets[0]->setSingleStep(0.001);
    inputOffsets[0]->setMinimum(0.05);
    inputOffsets[0]->setMaximum(3);

    checkOffset = new QCheckBox;

    radioLeftSide = new QRadioButton(tr("Left Offset"));
    radioRightSide = new QRadioButton(tr("Right Offset"));

    QGroupBox* groupOffset = new QGroupBox;
    {
        QGridLayout *grid = new QGridLayout;

        grid->addWidget(checkOffset, 0, 0);
        grid->addWidget(inputOffsets[0], 0, 1, 1, 2);
        grid->addWidget(radioLeftSide, 1, 1);
        grid->addWidget(radioRightSide, 1, 2);
        groupOffset->setLayout(grid);
    }
    groupOffset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    labelTimes = new QLabel(tr("Cut Times"));
//    comboTimes = new QComboBox;
//    labelTimes->setBuddy(comboTimes);

//    comboTimes->setEditable(true);
//    comboTimes->lineEdit()->setReadOnly(true);
//    comboTimes->lineEdit()->setAlignment(Qt::AlignCenter);

//    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
//        comboTimes->addItem(QString::number(i + 1));
//        comboTimes->setItemData(int(i), Qt::AlignCenter, Qt::TextAlignmentRole);
//    }

//    comboTimes->lineEdit()->setAlignment(Qt::AlignCenter);
//    comboTimes->setRange(1, int(par.cut.TIMES_MAX));
//    comboTimes->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    labelOverlap = new QLabel(tr("Overlap"));
    inputOverlap = new QDoubleSpinBox;
    labelOverlap->setBuddy(inputOverlap);
    inputOverlap->setSuffix(" " + tr("mm"));
    inputOverlap->setDecimals(1);
    inputOverlap->setSingleStep(1);
    inputOverlap->setMaximum(100);

    labelTab = new QLabel(tr("Tab width"));
    inputTab = new QDoubleSpinBox;
    labelTab->setBuddy(inputTab);
    inputTab->setSuffix(" mm");
    inputTab->setDecimals(1);
    inputTab->setSingleStep(1);
    inputTab->setMaximum(100);

    radioTabNone = new QRadioButton(tr("None"));
    radioTabLength = new QRadioButton;
    radioTabLast = new QRadioButton(tr("Last segment"));

    QGridLayout* gridTab = new QGridLayout;
    gridTab->addWidget(radioTabNone, 0, 0, 1, 2);
    gridTab->addWidget(radioTabLength, 1, 0);
    gridTab->addWidget(inputTab, 1, 1);
    gridTab->addWidget(radioTabLast, 2, 0, 1, 2);

    groupTab = new QGroupBox;
    groupTab->setLayout(gridTab);
    groupTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    labelModes = new QLabel*[par.cut.MAX_TIMES];
//    comboModes = new QComboBox*[par.cut.MAX_TIMES];

//    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
//        labelPasses[i] = new QLabel(tr("Offset in pass") + " " + QString::number(i + 1));
//        inputOffsets[i] = new QDoubleSpinBox;
//        labelPasses[i]->setBuddy(inputOffsets[i]);
//        inputOffsets[i]->setSuffix(" " + tr("mm"));
//        inputOffsets[i]->setDecimals(3);
//        inputOffsets[i]->setSingleStep(0.1);
//        inputOffsets[i]->setMaximum(2);
//        if (i > 0) inputOffset[i]->setEnabled(false);

//        labelModes[i] = new QLabel(tr("Mode in pass") + " " + QString::number(i + 1));
//        comboModes[i] = new QComboBox;
//        labelModes[i]->setBuddy(comboModes[i]);
//        comboModes[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        if (i > 0) comboMode[i]->setEnabled(false);
//    }

    // Tab
//    onePassTab = new QRadioButton(tr("Onepass Cutting"));
//    multiPassTab = new QRadioButton(tr("Multipass Cutting"));

//    groupTab = new QGroupBox(tr("Indent"));
//    groupTab->setLayout(new QHBoxLayout);
//    groupTab->layout()->addWidget(onePassTab);
//    groupTab->layout()->addWidget(multiPassTab);
//    groupTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    labelTabOffset = new QLabel(tr("Indent Offset"));
//    inputTabOffset = new QDoubleSpinBox;
//    labelTabOffset->setBuddy(inputTabOffset);
//    inputTabOffset->setSuffix(" " + tr("mm"));
//    inputTabOffset->setDecimals(3);
//    inputTabOffset->setSingleStep(0.1);
//    inputTabOffset->setMaximum(2);

//    labelCutMode = new QLabel(tr("Cutline Mode"));
//    comboCutMode = new QComboBox;
//    labelCutMode->setBuddy(comboCutMode);

//    labelTabMode = new QLabel(tr("Indent Mode"));
//    comboTabMode = new QComboBox;
//    labelTabMode->setBuddy(comboTabMode);

    checkTabPause = new QCheckBox(tr("Indent Pause"));
    checkTabPause->setEnabled(false);

//    labelPumpDelay = new QLabel(tr("Pump On Delay"));
//    inputPumpDelay = new QSpinBox;
//    labelPumpDelay->setBuddy(inputPumpDelay);
//    inputPumpDelay->setSuffix(" " + tr("sec"));
//    inputPumpDelay->setRange(30, 600);
//    inputPumpDelay->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    checkPumpPause = new QCheckBox(tr("Pump On Pause"));

    labelSpeed = new QLabel(tr("Cutting Speed"));
    inputSpeed = new QDoubleSpinBox;
    labelSpeed->setBuddy(inputSpeed);
    inputSpeed->setSuffix(" " + tr("mm/min"));
    inputSpeed->setDecimals(1);
    inputSpeed->setSingleStep(1);
    inputSpeed->setMinimum(0.1);
    inputSpeed->setMaximum(18);

    checkSegmentPause = new QCheckBox(tr("Add pause after each segment"));
    inputSegmentPause = new QSpinBox;
    inputSegmentPause->setSuffix(" " + tr("sec"));
    inputSegmentPause->setRange(1, 600);
    inputSegmentPause->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *layoutSegmentPause = new QHBoxLayout;
    layoutSegmentPause->addWidget(checkSegmentPause);
    layoutSegmentPause->addWidget(inputSegmentPause);

    gridPasses = new QGridLayout;

//    gridPasses->addWidget(labelTimes, 1, 0, Qt::AlignRight);
//    gridPasses->addWidget(comboTimes, 1, 1);

//    for (int i = 0; i < int(par.cut.MAX_TIMES); i++) {
//        gridPasses->addWidget(labelPasses[i], 2 + i, 0, Qt::AlignRight);
//        gridPasses->addWidget(inputOffsets[i], 2 + i, 1, Qt::AlignLeft);
//        gridPasses->addWidget(labelModes[i], 2 + i, 2, Qt::AlignRight);
//        gridPasses->addWidget(comboModes[i], 2 + i, 3, Qt::AlignLeft);
//    }

    gridPasses->addWidget(labelPasses[0], 0, 0, Qt::AlignRight);
    gridPasses->addWidget(groupOffset, 0, 1, Qt::AlignLeft);
    gridPasses->addWidget(labelAuxOffset, 1, 0, Qt::AlignRight);
    gridPasses->addWidget(groupAuxOffset, 1, 1, Qt::AlignLeft);

    QFrame* hLine[3];
    for (size_t i = 0; i < sizeof(hLine)/sizeof(hLine[0]); i++) {
        hLine[i] = new QFrame;
        hLine[i]->setFrameShape(QFrame::HLine);
        hLine[i]->setFrameShadow(QFrame::Sunken);
        hLine[i]->setLineWidth(3);
    }

//    gridPasses->addWidget(labelCutMode, 10, 2, Qt::AlignRight);
//    gridPasses->addWidget(comboCutMode, 10, 3, Qt::AlignLeft);

//    gridPasses->addWidget(hLine[1], 11, 1, 1, 3);

//    gridPasses->addWidget(groupTab, 12, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);

    gridPasses->addWidget(labelTab, 2, 0, Qt::AlignRight);
    gridPasses->addWidget(groupTab, 2, 1, 1, 2, Qt::AlignLeft);

//    gridPasses->addWidget(labelTabOffset, 14, 0, Qt::AlignRight);
//    gridPasses->addWidget(inputTabOffset, 14, 1, Qt::AlignLeft);
//    gridPasses->addWidget(labelTabMode, 14, 2, Qt::AlignRight);
//    gridPasses->addWidget(comboTabMode, 14, 3, Qt::AlignLeft);
    gridPasses->addWidget(checkTabPause, 3, 1, 1, 2, Qt::AlignLeft);

    gridPasses->addWidget(hLine[0], 4, 1, 1, 2);

    gridPasses->addWidget(labelOverlap, 5, 0, Qt::AlignRight);
    gridPasses->addWidget(inputOverlap, 5, 1, Qt::AlignLeft);

//    gridPasses->addWidget(hLine[1], 6, 1, 1, 2);

//    gridPasses->addWidget(labelPumpDelay, 7, 0, Qt::AlignLeft);
//    gridPasses->addWidget(inputPumpDelay, 7, 1, Qt::AlignLeft);
//    gridPasses->addWidget(checkPumpPause, 8, 1, 1, 2, Qt::AlignLeft);

    gridPasses->addWidget(hLine[1], 9, 1, 1, 3);
    gridPasses->addWidget(labelSpeed, 10, 0, Qt::AlignRight);
    gridPasses->addWidget(inputSpeed, 10, 1, Qt::AlignLeft);

    gridPasses->addWidget(hLine[2], 11, 1, 1, 3);
    gridPasses->addLayout(layoutSegmentPause, 12, 1, 1, 2, Qt::AlignRight);

    gridPasses->setSizeConstraint(QLayout::SetFixedSize);

//    gridPasses->setColumnStretch(0, 0);
//    gridPasses->setColumnStretch(1, 1);
//    gridPasses->setColumnStretch(2, 0);
//    gridPasses->setColumnStretch(3, 1);
//    gridPasses->setColumnStretch(4, 0);

//    gridPasses->setRowStretch(12, 1);

    initPasses();

    connect(checkAuxOffset, &QCheckBox::clicked, this, [&](bool checked) {
        par.cut.aux_offset_ena = checked;
        if (checked)
            par.cut.aux_offset = inputAuxOffset->value();
        else
            par.cut.aux_offset = 0;

        inputAuxOffset->setEnabled(checked);
        radioLeftAux->setEnabled(checked);
        radioRightAux->setEnabled(checked);
    });
    connect(inputAuxOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cut.aux_offset = value; });
    connect(radioLeftAux, &QRadioButton::clicked, this, [&]()   { par.cut.aux_offset_side = ProgramParam::swapXY ? OFFSET_SIDE::RIGHT : OFFSET_SIDE::LEFT; });
    connect(radioRightAux, &QRadioButton::clicked, this, [&]()  { par.cut.aux_offset_side = ProgramParam::swapXY ? OFFSET_SIDE::LEFT : OFFSET_SIDE::RIGHT; });

    connect(checkOffset, &QCheckBox::clicked, this, [&](bool checked) {
        par.cut.offset_ena = checked;

        for (offset_t& o: par.cut.offsets)
            if (checked)
                o.offset = inputOffsets[0]->value();
            else
                o.offset = 0;

        inputOffsets[0]->setEnabled(checked);
        radioLeftSide->setEnabled(checked);
        radioRightSide->setEnabled(checked);
    });
    connect(inputOffsets[0], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value) {
        this->par.cut.offsets[0].offset = value;
        par.cut.tab_offset.offset = value;
//        inputTabOffset->setValue(value);
    });
    connect(radioLeftSide, &QRadioButton::clicked, this, [&]()  { par.cut.offset_side = ProgramParam::swapXY ? OFFSET_SIDE::RIGHT : OFFSET_SIDE::LEFT; });
    connect(radioRightSide, &QRadioButton::clicked, this, [&]() { par.cut.offset_side = ProgramParam::swapXY ? OFFSET_SIDE::LEFT : OFFSET_SIDE::RIGHT; });

//    connect(comboTimes, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)                   { setCutTimes(value); });
//    connect(comboTimes, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) { setCutTimes(index + 1); });
    connect(inputOverlap, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)     { par.cut.overlap = value; });

    connect(radioTabNone, &QRadioButton::clicked, this, [&]() {
        par.cut.tab = 0;
        par.cut.tab_seg = 0;
        par.cut.tab_pause = false;
        inputTab->setEnabled(false);
        checkTabPause->setCheckState(Qt::CheckState::Unchecked);        
    });

    connect(radioTabLength, &QRadioButton::clicked, this, [&]() {
        par.cut.tab = inputTab->value();
        par.cut.tab_seg = 0;
        par.cut.tab_pause = true;
        inputTab->setEnabled(true);
        checkTabPause->setCheckState(Qt::CheckState::Checked);
    });

    connect(radioTabLast, &QRadioButton::clicked, this, [&]() {
        par.cut.tab = 0;
        par.cut.tab_seg = 1;
        par.cut.tab_pause = true;
        inputTab->setEnabled(false);
        checkTabPause->setCheckState(Qt::CheckState::Checked);
    });

    connect(inputTab, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)         { par.cut.tab = value; });

//    for (size_t i = 0; i < par.cut.offsets.size(); i++) {
//        connect(inputOffsets[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&, i](double value) {
//            this->par.cut.offsets[i].offset = value;

//            if (i == 0) {
//                par.cut.tab_offset.offset = value;
//                inputTabOffset->setValue(value);
//            }
//        });

//        connect(comboModes[i], QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&, i](int index) {
//            this->par.cut.offsets[i].mode_id = uint8_t(index);

//            if (i == 0) {
//                par.cut.tab_offset.mode_id = uint8_t(index);
//                comboTabMode->setCurrentIndex(index);
//            }
//        });
//    }

//    connect(onePassTab, &QRadioButton::clicked, this, [&]() {
//        par.cut.tab_multi_pass = false;
//        inputTabOffset->setEnabled(true);
//        comboTabMode->setEnabled(true);
//    });
//    connect(multiPassTab, &QRadioButton::clicked, this, [&]() {
//        par.cut.tab_multi_pass = true;
//        inputTabOffset->setEnabled(false);
//        comboTabMode->setEnabled(false);
//    });
//    connect(inputTabOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cut.tab_offset.offset = value; });
//    connect(comboTabMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)               {
//        par.cut.tab_offset.mode_id = uint8_t(index);
//    });
//    connect(comboCutMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)               {
//        par.cut.cutline_mode_id = uint8_t(index);
//    });
    connect(checkTabPause, &QCheckBox::clicked, this, [&](bool checked)                                     { par.cut.tab_pause = checked; });

//    connect(inputPumpDelay, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)               { par.cut.pump_delay = uint16_t(value); });
//    connect(checkPumpPause, &QCheckBox::clicked, this, [&](bool checked)                                    {
//        par.cut.pump_pause = checked;
//        inputPumpDelay->setEnabled(!checked);
//    });

    par.cut.pump_delay = 0;
    par.cut.pump_pause = false;

    connect(inputSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value) { par.cut.speed = value; });

    connect(checkSegmentPause, &QCheckBox::clicked, this, [&](bool checked) {
        par.cut.seg_pause = checked ? inputSegmentPause->value() : 0;
        inputSegmentPause->setEnabled(checked);
    });

    connect(inputSegmentPause, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
        par.cut.seg_pause = uint16_t(value);
    });
}

void FormPassesStone::initPasses() {
//    initComboMode();

    checkOffset->setCheckState(par.cut.offset_ena ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    inputOffsets[0]->setValue(par.cut.offsets[0].offset);
    inputOffsets[0]->setEnabled(par.cut.offset_ena);
    if (!par.cut.offset_ena)
        par.cut.offsets[0].offset = 0;

    bool rightChecked = ProgramParam::swapXY ? par.cut.offset_side == OFFSET_SIDE::RIGHT : par.cut.offset_side == OFFSET_SIDE::LEFT;
    radioLeftSide->setChecked(rightChecked);
    radioRightSide->setChecked(!rightChecked);
    radioLeftSide->setEnabled(par.cut.offset_ena);
    radioRightSide->setEnabled(par.cut.offset_ena);

    //
    checkAuxOffset->setCheckState(par.cut.aux_offset_ena ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    inputAuxOffset->setValue(par.cut.aux_offset);
    inputAuxOffset->setEnabled(par.cut.aux_offset_ena);
    if (!par.cut.aux_offset_ena)
        par.cut.aux_offset = 0;

    bool auxRightChecked = ProgramParam::swapXY ? par.cut.aux_offset_side == OFFSET_SIDE::RIGHT : par.cut.aux_offset_side == OFFSET_SIDE::LEFT;
    radioLeftAux->setChecked(auxRightChecked);
    radioRightAux->setChecked(!auxRightChecked);
    radioLeftAux->setEnabled(par.cut.aux_offset_ena);
    radioRightAux->setEnabled(par.cut.aux_offset_ena);

    qDebug("swapXY: %x offset_side: %x (right: %x) aux_offset_side: %x (aux_right: %x)\n",
           int(ProgramParam::swapXY), int(par.cut.offset_side), int(rightChecked), int(par.cut.aux_offset_side), int(auxRightChecked));

//    comboTimes->setValue(par.cut.times);
    int index = par.cut.times - 1;
    if (index < 0)
        index = 0;
    else if (index > int(par.cut.MAX_TIMES) - 1)
        index = par.cut.MAX_TIMES - 1;

//    comboTimes->setCurrentIndex(par.cut.times - 1 );
//    setCutTimes(par.cut.times);

    inputOverlap->setValue(par.cut.overlap);

//    for (size_t i = 0; i < par.cut.offsets.size(); i++) {
//        if (i < par.cut.offsets.size()) {
//            inputOffsets[i]->setValue(par.cut.offsets[i].offset);
//            comboModes[i]->setCurrentIndex(par.cut.offsets[i].mode_id);
//        }
//        else {
////            inputOffset[i]->setValue(0);
////            comboMode[i]->setCurrentIndex(0);
//        }
//    }

//    comboCutMode->setCurrentIndex(par.cut.cutline_mode_id);

//    onePassTab->setChecked(!par.cut.tab_multi_pass);
//    multiPassTab->setChecked(par.cut.tab_multi_pass);

//    inputTabOffset->setEnabled(!par.cut.tab_multi_pass);
//    comboTabMode->setEnabled(!par.cut.tab_multi_pass);

//    inputTabOffset->setValue(par.cut.tab_offset.offset);
//    comboTabMode->setCurrentIndex(par.cut.tab_offset.mode_id);

    inputTab->setEnabled(false);
    inputTab->setValue(par.cut.tab);

    if (par.cut.tab_pause) {
        if (par.cut.tab_seg)
            radioTabLast->setChecked(true);
        else {
            radioTabLength->setChecked(true);
            inputTab->setEnabled(true);
        }
        checkTabPause->setCheckState(Qt::CheckState::Checked);
    }
    else {
        radioTabNone->setChecked(true);
        checkTabPause->setCheckState(Qt::CheckState::Unchecked);
    }

//    inputPumpDelay->setValue(par.cut.pump_delay);
//    inputPumpDelay->setEnabled(!par.cut.pump_pause);
//    checkPumpPause->setCheckState(par.cut.pump_pause ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    inputSpeed->setValue(par.cut.speed);

    inputSegmentPause->setValue(par.cut.seg_pause ? par.cut.seg_pause : 30);
    inputSegmentPause->setEnabled(par.cut.seg_pause != 0);
    checkSegmentPause->setCheckState(par.cut.seg_pause != 0 ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

//void FormStoneOffset::resizeModeView() {
//    tableMode->resizeColumnsToContents();

//    int width = 0;
//    for (int i = 0; i < tableMode->model()->columnCount(); i++)
//        width += tableMode->columnWidth(i);

//    width += tableMode->verticalHeader()->width() + tableMode->columnWidth(0) / 4;

//    int height = tableMode->rowHeight(0) * 10;

//    tableMode->setFixedSize(width, height);
//}

//void FormStoneOffset::createModeView() {
//    tableTitle = new QLabel(R"(<h3>)" + tr("Generator Operation Modes") + R"(</h3>)");

//    tableMode = new QTableView;

//    tableMode->setModel(new ModeTableModel(par.genModeList, this));
//    tableMode->setItemDelegate(new SpinBoxDelegate(this));
//    resizeModeView();

//    tableMode->horizontalHeader()->setStretchLastSection(true);

////    tableMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    btnNewMode = new QPushButton(tr("New"));
//    btnDeleteMode = new QPushButton(tr("Delete"));
//    btnOpenModes = new QPushButton(tr("Open"));
//    btnSaveModes = new QPushButton(tr("Save"));
//    btnSaveModes->setEnabled(false);
//    btnSaveAsModes = new QPushButton(tr("Save As"));
//    btnDefaultModes = new QPushButton(tr("Factory"));

//    vboxEdit = new QVBoxLayout;
//    vboxEdit->addWidget(btnNewMode);
//    vboxEdit->addWidget(btnDeleteMode);
//    vboxEdit->addWidget(btnOpenModes);
//    vboxEdit->addWidget(btnSaveModes);
//    vboxEdit->addWidget(btnSaveAsModes);
//    vboxEdit->addWidget(btnDefaultModes);
////    vboxEdit->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
//    vboxEdit->setSizeConstraint(QLayout::SetFixedSize);

//    gridMode = new QGridLayout;
//    gridMode->addWidget(tableTitle, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);

//    gridMode->addWidget(tableMode, 1, 0, Qt::AlignLeft | Qt::AlignTop);
//    gridMode->addLayout(vboxEdit, 1, 1, Qt::AlignLeft | Qt::AlignTop);
//    gridMode->setSizeConstraint(QLayout::SetFixedSize);

////    gridMode->setColumnStretch(0, 0);
////    gridMode->setColumnStretch(1, 1);
////    gridMode->setColumnStretch(2, 1);

//    connect(tableMode, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onTableModeClicked(const QModelIndex&)));

//    connect(btnNewMode, &QPushButton::clicked, this, [&]() {
//        tableMode->model()->insertRow(int(par.genModeList.size()));
//        initComboMode();
//        unbindModesFile();
//    });

//    connect(btnOpenModes, &QPushButton::clicked, this, &FormPasses::on_btnOpenModes_clicked);
//    connect(btnSaveModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveModes_clicked);
//    connect(btnSaveAsModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveAsModes_clicked);

//    connect(btnDeleteMode, &QPushButton::clicked, this, [&]() {
//        tableMode->model()->removeRow(m_row);
//        initComboMode();
//    });

//    connect(btnDefaultModes, &QPushButton::clicked, this, [&]() {
//        QItemSelectionModel* m = tableMode->selectionModel();

//        par.setDefaultGenModeList();
//        tableMode->setModel(new ModeTableModel(par.genModeList, this));
//        delete m;

//        tableMode->resizeColumnsToContents();

//        initComboMode();

//        unbindModesFile();
//    });
//}

//void FormStoneOffset::bindModesFile() { btnSaveModes->setEnabled(true); }
//void FormStoneOffset::unbindModesFile() { btnSaveModes->setEnabled(false); }
//bool FormStoneOffset::bindedModesFile() const { return btnSaveModes->isEnabled(); }

//void FormStoneOffset::on_btnOpenModes_clicked() {
//    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open modes file"),
//                               par.fileDir + "/" + par.modesFileName,
//                               tr("Modes file (*.xmd *.XMD)"));

//    if (new_filePath.length() != 0) {
//        QFileInfo fInfo(new_filePath);
//        par.fileDir = fInfo.dir().path();
//        par.modesFileName = fInfo.fileName();

//        QFile file(par.fileDir + "/" + par.modesFileName);

//        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//            QMessageBox::critical(this, tr("Modes: Open File Error"), file.errorString());
//        else {
//            GeneratorModeList list(par.genModeList);
//            list.openXML(file);

//            if (list.hasError())
//                QMessageBox::critical(this, tr("Modes: Parse File Error"), list.errorString(), QMessageBox::Ok);
//            else {
//                QItemSelectionModel* m = tableMode->selectionModel();

//                par.genModeList = list.get();
//                tableMode->setModel(new ModeTableModel(par.genModeList, this));

//                delete m;

//                tableMode->resizeColumnsToContents();

//                initComboMode();

//                bindModesFile();
//            }

//            file.close();
//        }

////            par.saveSettings();
//    }
//}

//void FormStoneOffset::on_btnSaveModes_clicked() {
//    QDir dir(par.fileDir);
//    if (!dir.exists())
//        par.fileDir.clear();

//    if (!bindedModesFile() || par.fileDir.length() == 0 || par.modesFileName.length() == 0)
//        on_btnSaveAsModes_clicked();
//    else {
//        QFile file(par.fileDir + "/" + par.modesFileName);

//        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//            QMessageBox::critical(this, tr("Modes: Save File Error"), file.errorString());
//        else {
//            GeneratorModeList list(par.genModeList);
//            list.saveXML(file);
//            file.close();
//        }
//    }
//}

//void FormStoneOffset::on_btnSaveAsModes_clicked() {
//    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save modes file"),
//                               par.fileDir + "/" + par.modesFileName,
//                               tr("Modes file (*.xmd *.XMD)"));

//    if (new_filePath.length() != 0) {
//        QFileInfo fInfo(new_filePath);
//        par.fileDir = fInfo.dir().path();
//        par.modesFileName = fInfo.fileName();
//        bindModesFile();
//        on_btnSaveModes_clicked();
////            par.saveSettings();
//    }
//}

void FormPassesStone::on_btnOpen_clicked() {

}

void FormPassesStone::on_btnSave_clicked() {
    QDir dir(par.fileDir);
    if (!dir.exists())
        par.fileDir.clear();

//    if (!bindedModesFile() || par.fileDir.length() == 0 || par.modesFileName.length() == 0)
    if (par.fileDir.length() == 0 || par.cutFileName.length() == 0)
        on_btnSaveAs_clicked();
    else {
        QFile file(par.fileDir + "/" + par.cutFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Cutting: Save File Error"), file.errorString());
        else {
            WireCut cut(par.cut);
            cut.saveXML(file, GeneratorModeList(par.genModesList));
            file.close();
        }
    }
}

void FormPassesStone::on_btnSaveAs_clicked() {
    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save cutting settings"),
                               par.fileDir + "/" + par.cutFileName,
                               tr("Cutting settings files") + " (*.xct *.XCT)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.fileDir = fInfo.dir().path();
        par.cutFileName = fInfo.fileName();
//        bindModesFile();
        on_btnSave_clicked();
//            par.saveSettings();
    }
}


void FormPassesStone::createButtons() {
    btnBack = new QPushButton(tr("Back"));
    btnBack->setStatusTip(tr("Return to the contour panel"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setEnabled(false);
    btnOpen->setStatusTip(tr("Open contour settings from a file"));

    btnSave = new QPushButton(tr("Save"));
    btnSave->setEnabled(false);
    btnSave->setStatusTip(tr("Save contour settings"));

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setEnabled(false);
    btnSaveAs->setStatusTip(tr("Save contour settings into a new file"));

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setStatusTip(tr("Set default values"));

    btn5 = new QPushButton;
    btn5->setEnabled(false);

    btn6 = new QPushButton;
    btn6->setEnabled(false);

    btn7 = new QPushButton;
    btn7->setEnabled(false);

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btn12 = new QPushButton;
    btn12->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnBack, 0, 0);    
    gridButtons->addWidget(btnOpen, 0, 1);
    gridButtons->addWidget(btnSave, 0, 2);
    gridButtons->addWidget(btnSaveAs, 0, 3);
    gridButtons->addWidget(btnDefault, 0, 4);
    gridButtons->addWidget(btn5, 0, 5);
    gridButtons->addWidget(btn6, 0, 6);
    gridButtons->addWidget(btn7, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnBack, &QPushButton::clicked, this, [&]() { emit backPageClicked(); });
    connect(btnOpen, &QPushButton::clicked, this, &FormPassesStone::on_btnOpen_clicked);
    connect(btnSave, &QPushButton::clicked, this, &FormPassesStone::on_btnSave_clicked);
    connect(btnSaveAs, &QPushButton::clicked, this, &FormPassesStone::on_btnSaveAs_clicked);
    connect(btnDefault, &QPushButton::clicked, this, [&]() {
        par.setDefaultCutParam();        
        initPasses();
    });

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

//void FormStoneOffset::initComboMode() {
//    QStringList newText;

//    for (size_t i = 0; i < par.genModeList.size(); i++)
//        newText.push_back(tr("Mode") + " " + QString::number(i + 1));

//    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
//        comboModes[i]->clear();
//        comboModes[i]->addItems(newText);
//    }

//    comboTabMode->clear();
//    comboTabMode->addItems(newText);

//    comboCutMode->clear();
//    comboCutMode->addItems(newText);
//}

//void FormStoneOffset::setCutTimes(int times) {
//    par.cut.times = (times > 0) ? uint8_t(times) : 1;

//    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
//        if (i < par.cut.times) {
//            inputOffsets[i]->setEnabled(true);
//            comboModes[i]->setEnabled(true);
//        }
//        else {
//            inputOffsets[i]->setEnabled(false);
//            comboModes[i]->setEnabled(false);
//        }
//    }
//}

//void FormStoneOffset::onTableModeClicked(const QModelIndex& index) {
//    if (index.isValid())
//        m_row = index.row() < 0 ? 0 : index.row();
//}

FormPassesStone::~FormPassesStone() {
    if (labelPasses) {
        delete[] labelPasses;
        labelPasses = nullptr;
    }
//    delete[] labelModes;
    if (inputOffsets) {
        delete[] inputOffsets;
        inputOffsets = nullptr;
    }
//    delete[] comboModes;
}

void FormPassesStone::setFontPointSize(int pointSize) {
    QFont font;

    for (QPushButton* o: buttons) {
        if (!o) continue;

        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QLabel* o: labels) {
        if (!o) continue;

        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

//    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
//        font = labelPasses[i]->font();
//        font.setPointSize(pointSize);
//        labelPasses[i]->setFont(font);

//        font = labelModes[i]->font();
//        font.setPointSize(pointSize);
//        labelModes[i]->setFont(font);

//        font = comboModes[i]->font();
//        font.setPointSize(pointSize);
//        comboModes[i]->setFont(font);

//        font = inputOffsets[i]->font();
//        font.setPointSize(pointSize);
//        inputOffsets[i]->setFont(font);
//    }

    font = labelPasses[0]->font();
    font.setPointSize(pointSize);
    labelPasses[0]->setFont(font);

    font = inputOffsets[0]->font();
    font.setPointSize(pointSize);
    inputOffsets[0]->setFont(font);

    for (QRadioButton* o: radio) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QComboBox* o: combo) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QCheckBox* o: checks) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QDoubleSpinBox* o: nums) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

//    font = inputPumpDelay->font();
//    font.setPointSize(pointSize);
//    inputPumpDelay->setFont(font);

//    font = groupTab->font();
//    font.setPointSize(pointSize);
//    groupTab->setFont(font);

//    font = tableMode->font();
//    font.setPointSize(pointSize);
//    tableMode->setFont(font);
//    resizeModeView();

    font = inputSegmentPause->font();
    font.setPointSize(pointSize);
    inputSegmentPause->setFont(font);
}
