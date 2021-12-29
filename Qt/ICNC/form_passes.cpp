#include "form_passes.h"
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

FormPasses::FormPasses(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par), m_pass_num(1), m_row(0) {
    QLabel* labelTitle = new QLabel(R"(<h1>)" + tr("Cutting settings") + R"(</h1>)");
    labelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    createPasses();
    createModeView();
    createButtons();

    buttons = {btnBack, btnOpen, btnSave, btnSaveAs, btnDefault, btn5, btn6, btn7, btn8, btn9, btn10, btn11, btn12, btnHelp,
               btnNewMode, btnDeleteMode, btnOpenModes, btnSaveModes, btnSaveAsModes, btnDefaultModes
              };
    labels = {labelTimes, labelOverlap, labelTab, labelTabOffset, labelTabMode, labelCutMode, labelPumpDelay, tableTitle};
    radio = {leftSide, rightSide, onePassTab, multiPassTab};
    combo = {comboTimes, comboCutMode, comboTabMode};
    checks = {checkTabPause, checkPumpPause};
    nums = {inputOverlap, inputTab, inputTabOffset};

//    setFontPointSize(14);

    gridPassMode = new QGridLayout;

    gridPassMode->addWidget(labelTitle, 0, 0, 1, 3, Qt::AlignHCenter);
    gridPassMode->addLayout(gridPasses, 1, 0, Qt::AlignVCenter | Qt::AlignRight);
//    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Preferred), 1, 1);
    gridPassMode->addItem(new QSpacerItem(32, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 1);
    gridPassMode->addLayout(gridMode, 1, 2, Qt::AlignVCenter | Qt::AlignLeft);

    gridPassMode->setColumnStretch(0, 1);
    gridPassMode->setColumnStretch(1, 0);
    gridPassMode->setColumnStretch(2, 1);

    gridPassMode->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridPassMode);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

void FormPasses::createPasses() {
    leftSide = new QRadioButton(tr("Left Offset"));
    rightSide = new QRadioButton(tr("Right Offset"));

    groupSide = new QGroupBox;
    groupSide->setLayout(new QHBoxLayout);
    groupSide->layout()->addWidget(leftSide);
    groupSide->layout()->addWidget(rightSide);

    groupSide->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelTimes = new QLabel(tr("Cut Times"));
    comboTimes = new QComboBox;
    labelTimes->setBuddy(comboTimes);

    comboTimes->setEditable(true);
    comboTimes->lineEdit()->setReadOnly(true);
    comboTimes->lineEdit()->setAlignment(Qt::AlignCenter);

    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
        comboTimes->addItem(QString::number(i + 1));
        comboTimes->setItemData(int(i), Qt::AlignCenter, Qt::TextAlignmentRole);
    }

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

    labelTab = new QLabel(tr("Indent Width"));
    inputTab = new QDoubleSpinBox;
    labelTab->setBuddy(inputTab);
    inputTab->setSuffix(" mm");
    inputTab->setDecimals(1);
    inputTab->setSingleStep(1);
    inputTab->setMaximum(100);

    labelPasses = new QLabel*[par.cut.MAX_TIMES];
    inputOffsets = new QDoubleSpinBox*[par.cut.MAX_TIMES];

    labelModes = new QLabel*[par.cut.MAX_TIMES];
    comboModes = new QComboBox*[par.cut.MAX_TIMES];

    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
        labelPasses[i] = new QLabel(tr("Offset in pass") + " " + QString::number(i + 1));
        inputOffsets[i] = new QDoubleSpinBox;
        labelPasses[i]->setBuddy(inputOffsets[i]);
        inputOffsets[i]->setSuffix(" " + tr("mm"));
        inputOffsets[i]->setDecimals(3);
        inputOffsets[i]->setSingleStep(0.1);
        inputOffsets[i]->setMaximum(2);
//        if (i > 0) inputOffset[i]->setEnabled(false);

        labelModes[i] = new QLabel(tr("Mode in pass") + " " + QString::number(i + 1));
        comboModes[i] = new QComboBox;
        labelModes[i]->setBuddy(comboModes[i]);
        comboModes[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        if (i > 0) comboMode[i]->setEnabled(false);
    }

    // Tab
    onePassTab = new QRadioButton(tr("Onepass"));
    multiPassTab = new QRadioButton(tr("Multipass"));

    groupTab = new QGroupBox(tr("Indent"));
    groupTab->setLayout(new QHBoxLayout);
    groupTab->layout()->addWidget(onePassTab);
    groupTab->layout()->addWidget(multiPassTab);
    groupTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    labelTabOffset = new QLabel(tr("Indent Offset"));
    inputTabOffset = new QDoubleSpinBox;
    labelTabOffset->setBuddy(inputTabOffset);
    inputTabOffset->setSuffix(" " + tr("mm"));
    inputTabOffset->setDecimals(3);
    inputTabOffset->setSingleStep(0.1);
    inputTabOffset->setMaximum(2);

    labelCutMode = new QLabel(tr("Cutline Mode"));
    comboCutMode = new QComboBox;
    labelCutMode->setBuddy(comboCutMode);

    labelTabMode = new QLabel(tr("Indent Mode"));
    comboTabMode = new QComboBox;
    labelTabMode->setBuddy(comboTabMode);

    checkTabPause = new QCheckBox(tr("Indent Pause"));

    labelPumpDelay = new QLabel(tr("Pump On Pause"));
    inputPumpDelay = new QSpinBox;
    labelPumpDelay->setBuddy(inputPumpDelay);
    inputPumpDelay->setSuffix(" " + tr("sec"));
    inputPumpDelay->setRange(30, 600);
    inputPumpDelay->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    checkPumpPause = new QCheckBox(tr("Pump On Stop"));

    gridPasses = new QGridLayout;

    gridPasses->addWidget(groupSide, 0, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);
    gridPasses->addWidget(labelTimes, 1, 0, Qt::AlignRight);
    gridPasses->addWidget(comboTimes, 1, 1);

    for (int i = 0; i < int(par.cut.MAX_TIMES); i++) {
        gridPasses->addWidget(labelPasses[i], 2 + i, 0, Qt::AlignRight);
        gridPasses->addWidget(inputOffsets[i], 2 + i, 1, Qt::AlignLeft);
        gridPasses->addWidget(labelModes[i], 2 + i, 2, Qt::AlignRight);
        gridPasses->addWidget(comboModes[i], 2 + i, 3, Qt::AlignLeft);
    }

    QFrame* hLine[4];
    for (size_t i = 0; i < sizeof(hLine)/sizeof(hLine[0]); i++) {
        hLine[i] = new QFrame;
        hLine[i]->setFrameShape(QFrame::HLine);
        hLine[i]->setFrameShadow(QFrame::Sunken);
        hLine[i]->setLineWidth(3);
    }

    gridPasses->addWidget(hLine[0], 9, 1, 1, 3);

    gridPasses->addWidget(labelCutMode, 10, 2, Qt::AlignRight);
    gridPasses->addWidget(comboCutMode, 10, 3, Qt::AlignLeft);

    gridPasses->addWidget(hLine[1], 11, 1, 1, 3);

    gridPasses->addWidget(groupTab, 12, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);

    gridPasses->addWidget(labelTab, 13, 0, Qt::AlignRight);
    gridPasses->addWidget(inputTab, 13, 1, Qt::AlignLeft);

    gridPasses->addWidget(labelTabOffset, 14, 0, Qt::AlignRight);
    gridPasses->addWidget(inputTabOffset, 14, 1, Qt::AlignLeft);
    gridPasses->addWidget(labelTabMode, 14, 2, Qt::AlignRight);
    gridPasses->addWidget(comboTabMode, 14, 3, Qt::AlignLeft);
    gridPasses->addWidget(checkTabPause, 15, 1, 1, 3, Qt::AlignLeft);

    gridPasses->addWidget(hLine[2], 16, 1, 1, 3);

    gridPasses->addWidget(labelOverlap, 17, 0, Qt::AlignRight);
    gridPasses->addWidget(inputOverlap, 17, 1, Qt::AlignLeft);

    gridPasses->addWidget(hLine[3], 18, 1, 1, 3);

    gridPasses->addWidget(labelPumpDelay, 19, 0, Qt::AlignLeft);
    gridPasses->addWidget(inputPumpDelay, 19, 1, Qt::AlignLeft);
    gridPasses->addWidget(checkPumpPause, 20, 1, 1, 3, Qt::AlignLeft);

    gridPasses->setSizeConstraint(QLayout::SetFixedSize);

//    gridPasses->setColumnStretch(0, 0);
//    gridPasses->setColumnStretch(1, 1);
//    gridPasses->setColumnStretch(2, 0);
//    gridPasses->setColumnStretch(3, 1);
//    gridPasses->setColumnStretch(4, 0);

//    gridPasses->setRowStretch(12, 1);

    initPasses();

    connect(leftSide, &QRadioButton::clicked, this, [&]()                                                   { par.cut.offset_side = OFFSET_SIDE::LEFT; });
    connect(rightSide, &QRadioButton::clicked, this, [&]()                                                  { par.cut.offset_side = OFFSET_SIDE::RIGHT; });
//    connect(comboTimes, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)                   { setCutTimes(value); });
    connect(comboTimes, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) { setCutTimes(index + 1); });
    connect(inputOverlap, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)     { par.cut.overlap = value; });
    connect(inputTab, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)         { par.cut.tab = value; });

    for (size_t i = 0; i < par.cut.offsets.size(); i++) {
        connect(inputOffsets[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&, i](double value) {
            this->par.cut.offsets[i].offset = value;

            if (i == 0) {
                par.cut.tab_offset.offset = value;
                inputTabOffset->setValue(value);
            }
        });
        connect(comboModes[i], QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&, i](int index) {
            this->par.cut.offsets[i].mode_id = uint8_t(index);

            if (i == 0) {
                par.cut.tab_offset.mode_id = uint8_t(index);
                comboTabMode->setCurrentIndex(index);
            }
        });
    }

    connect(onePassTab, &QRadioButton::clicked, this, [&]() {
        par.cut.tab_multi_pass = false;
        inputTabOffset->setEnabled(true);
        comboTabMode->setEnabled(true);
    });
    connect(multiPassTab, &QRadioButton::clicked, this, [&]() {
        par.cut.tab_multi_pass = true;
        inputTabOffset->setEnabled(false);
        comboTabMode->setEnabled(false);
    });
    connect(inputTabOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value)   { par.cut.tab_offset.offset = value; });
    connect(comboTabMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)               {
        par.cut.tab_offset.mode_id = uint8_t(index);
    });
    connect(comboCutMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index)               {
        par.cut.cutline_mode_id = uint8_t(index);
    });
    connect(checkTabPause, &QCheckBox::clicked, this, [&](bool checked)                                     { par.cut.tab_pause = checked; });

    connect(inputPumpDelay, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value)               { par.cut.pump_delay = uint16_t(value); });
    connect(checkPumpPause, &QCheckBox::clicked, this, [&](bool checked)                                    { par.cut.pump_pause = checked; });
}

void FormPasses::initPasses() {
    initComboMode();

    leftSide->setChecked(par.cut.offset_side == OFFSET_SIDE::LEFT);
    leftSide->setChecked(par.cut.offset_side == OFFSET_SIDE::RIGHT);

//    comboTimes->setValue(par.cut.times);
    int index = par.cut.times - 1;
    if (index < 0)
        index = 0;
    else if (index > int(par.cut.MAX_TIMES) - 1)
        index = par.cut.MAX_TIMES - 1;

    comboTimes->setCurrentIndex(par.cut.times - 1 );
    setCutTimes(par.cut.times);

    inputOverlap->setValue(par.cut.overlap);
    inputTab->setValue(par.cut.tab);

    for (size_t i = 0; i < par.cut.offsets.size(); i++) {
        if (i < par.cut.offsets.size()) {
            inputOffsets[i]->setValue(par.cut.offsets[i].offset);
            comboModes[i]->setCurrentIndex(par.cut.offsets[i].mode_id);
        }
        else {
//            inputOffset[i]->setValue(0);
//            comboMode[i]->setCurrentIndex(0);
        }
    }

    comboCutMode->setCurrentIndex(par.cut.cutline_mode_id);

    onePassTab->setChecked(!par.cut.tab_multi_pass);
    multiPassTab->setChecked(par.cut.tab_multi_pass);

    inputTabOffset->setEnabled(!par.cut.tab_multi_pass);
    comboTabMode->setEnabled(!par.cut.tab_multi_pass);

    inputTabOffset->setValue(par.cut.tab_offset.offset);
    comboTabMode->setCurrentIndex(par.cut.tab_offset.mode_id);
    checkTabPause->setCheckState((par.cut.tab_pause) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    inputPumpDelay->setValue(par.cut.pump_delay);
    checkPumpPause->setCheckState((par.cut.pump_pause) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

void FormPasses::resizeModeView() {
    tableMode->resizeColumnsToContents();

    int width = 0;
    for (int i = 0; i < tableMode->model()->columnCount(); i++)
        width += tableMode->columnWidth(i);

    width += tableMode->verticalHeader()->width() + tableMode->columnWidth(0) / 4;

    int height = tableMode->rowHeight(0) * 10;

    tableMode->setFixedSize(width, height);
}

void FormPasses::createModeView() {
    tableTitle = new QLabel(R"(<h3>)" + tr("Generator Operation Modes") + R"(</h3>)");

    tableMode = new QTableView;

    tableMode->setModel(new ModeTableModel(par.genModesList, this));
    tableMode->setItemDelegate(new SpinBoxDelegate(this));
    resizeModeView();

    tableMode->horizontalHeader()->setStretchLastSection(true);

//    tableMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    btnNewMode = new QPushButton(tr("New"));
    btnDeleteMode = new QPushButton(tr("Delete"));
    btnOpenModes = new QPushButton(tr("Open"));
    btnSaveModes = new QPushButton(tr("Save"));
    btnSaveModes->setEnabled(false);
    btnSaveAsModes = new QPushButton(tr("Save As"));
    btnDefaultModes = new QPushButton(tr("Default"));

    vboxEdit = new QVBoxLayout;
    vboxEdit->addWidget(btnNewMode);
    vboxEdit->addWidget(btnDeleteMode);
    vboxEdit->addWidget(btnOpenModes);
    vboxEdit->addWidget(btnSaveModes);
    vboxEdit->addWidget(btnSaveAsModes);
    vboxEdit->addWidget(btnDefaultModes);
//    vboxEdit->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vboxEdit->setSizeConstraint(QLayout::SetFixedSize);

    gridMode = new QGridLayout;
    gridMode->addWidget(tableTitle, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);

    gridMode->addWidget(tableMode, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    gridMode->addLayout(vboxEdit, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    gridMode->setSizeConstraint(QLayout::SetFixedSize);

//    gridMode->setColumnStretch(0, 0);
//    gridMode->setColumnStretch(1, 1);
//    gridMode->setColumnStretch(2, 1);

    connect(tableMode, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onTableModeClicked(const QModelIndex&)));

    connect(btnNewMode, &QPushButton::clicked, this, [&]() {
        tableMode->model()->insertRow(int(par.genModesList.size()));
        initComboMode();
        unbindModesFile();
    });

    connect(btnOpenModes, &QPushButton::clicked, this, &FormPasses::on_btnOpenModes_clicked);
    connect(btnSaveModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveModes_clicked);
    connect(btnSaveAsModes, &QPushButton::clicked, this, &FormPasses::on_btnSaveAsModes_clicked);

    connect(btnDeleteMode, &QPushButton::clicked, this, [&]() {
        tableMode->model()->removeRow(m_row);
        initComboMode();
    });

    connect(btnDefaultModes, &QPushButton::clicked, this, [&]() {
        QItemSelectionModel* m = tableMode->selectionModel();

        par.setDefaultGenModeList();
        tableMode->setModel(new ModeTableModel(par.genModesList, this));
        delete m;

        tableMode->resizeColumnsToContents();

        initComboMode();

        unbindModesFile();
    });
}

void FormPasses::bindModesFile() { btnSaveModes->setEnabled(true); }
void FormPasses::unbindModesFile() { btnSaveModes->setEnabled(false); }
bool FormPasses::bindedModesFile() const { return btnSaveModes->isEnabled(); }

void FormPasses::on_btnOpenModes_clicked() {
    QString new_filePath = QFileDialog::getOpenFileName(this, tr("Open modes file"),
                               par.fileDir + "/" + par.modesFileName,
                               tr("Modes files") + " (*.xmd *.XMD)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.fileDir = fInfo.dir().path();
        par.modesFileName = fInfo.fileName();

        QFile file(par.fileDir + "/" + par.modesFileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Modes: Open File Error"), file.errorString());
        else {
            GeneratorModeList list(par.genModesList);
            list.openXML(file);

            if (list.hasError())
                QMessageBox::critical(this, tr("Modes: Parse File Error"), list.errorString(), QMessageBox::Ok);
            else {
                QItemSelectionModel* m = tableMode->selectionModel();

                par.genModesList = list.get();
                tableMode->setModel(new ModeTableModel(par.genModesList, this));

                delete m;

                tableMode->resizeColumnsToContents();

                initComboMode();

                bindModesFile();
            }

            file.close();
        }

//            par.saveSettings();
    }
}

void FormPasses::on_btnSaveModes_clicked() {
    QDir dir(par.fileDir);
    if (!dir.exists())
        par.fileDir.clear();

    if (!bindedModesFile() || par.fileDir.length() == 0 || par.modesFileName.length() == 0)
        on_btnSaveAsModes_clicked();
    else {
        QFile file(par.fileDir + "/" + par.modesFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            QMessageBox::critical(this, tr("Modes: Save File Error"), file.errorString());
        else {
            GeneratorModeList list(par.genModesList);
            list.saveXML(file);
            file.close();
        }
    }
}

void FormPasses::on_btnSaveAsModes_clicked() {
    QString new_filePath = QFileDialog::getSaveFileName(this, tr("Save modes file"),
                               par.fileDir + "/" + par.modesFileName,
                               tr("Modes files") + " (*.xmd *.XMD)");

    if (new_filePath.length() != 0) {
        QFileInfo fInfo(new_filePath);
        par.fileDir = fInfo.dir().path();
        par.modesFileName = fInfo.fileName();
        bindModesFile();
        on_btnSaveModes_clicked();
//            par.saveSettings();
    }
}

void FormPasses::on_btnOpen_clicked() {

}

void FormPasses::on_btnSave_clicked() {
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

void FormPasses::on_btnSaveAs_clicked() {
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


void FormPasses::createButtons() {
    btnBack = new QPushButton(tr("Back"));
    btnBack->setStatusTip(tr("Back to contour panel"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setEnabled(true);

    btnSave = new QPushButton(tr("Save"));
    btnSave->setEnabled(false);

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setEnabled(true);

    btnDefault = new QPushButton(tr("Default"));
    btnDefault->setEnabled(false);
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
    connect(btnOpen, &QPushButton::clicked, this, &FormPasses::on_btnOpen_clicked);
    connect(btnSave, &QPushButton::clicked, this, &FormPasses::on_btnSave_clicked);
    connect(btnSaveAs, &QPushButton::clicked, this, &FormPasses::on_btnSaveAs_clicked);

    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormPasses::initComboMode() {
    QStringList newText;

    for (size_t i = 0; i < par.genModesList.size(); i++)
        newText.push_back(tr("Mode") + " " + QString::number(i + 1));

    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
        comboModes[i]->clear();
        comboModes[i]->addItems(newText);
    }

    comboTabMode->clear();
    comboTabMode->addItems(newText);

    comboCutMode->clear();
    comboCutMode->addItems(newText);
}

void FormPasses::setCutTimes(int times) {
    par.cut.times = (times > 0) ? uint8_t(times) : 1;

    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
        if (i < par.cut.times) {
            inputOffsets[i]->setEnabled(true);
            comboModes[i]->setEnabled(true);
        }
        else {
            inputOffsets[i]->setEnabled(false);
            comboModes[i]->setEnabled(false);
        }
    }
}

void FormPasses::onTableModeClicked(const QModelIndex& index) {
    if (index.isValid())
        m_row = index.row() < 0 ? 0 : index.row();
}

FormPasses::~FormPasses() {
    if (labelPasses) {
        delete[] labelPasses;
        labelPasses = nullptr;
    }
    if (labelModes) {
        delete[] labelModes;
        labelModes = nullptr;
    }
    if (inputOffsets) {
        delete[] inputOffsets;
        inputOffsets = nullptr;
    }
    if (comboModes) {
        delete[] comboModes;
        comboModes = nullptr;
    }
}

void FormPasses::setFontPointSize(int pointSize) {
    QFont font;

    for (QPushButton* o: buttons) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (QLabel* o: labels) {
        font = o->font();
        font.setPointSize(pointSize);
        o->setFont(font);
    }

    for (size_t i = 0; i < par.cut.MAX_TIMES; i++) {
        font = labelPasses[i]->font();
        font.setPointSize(pointSize);
        labelPasses[i]->setFont(font);

        font = labelModes[i]->font();
        font.setPointSize(pointSize);
        labelModes[i]->setFont(font);

        font = comboModes[i]->font();
        font.setPointSize(pointSize);
        comboModes[i]->setFont(font);

        font = inputOffsets[i]->font();
        font.setPointSize(pointSize);
        inputOffsets[i]->setFont(font);
    }

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

    font = inputPumpDelay->font();
    font.setPointSize(pointSize);
    inputPumpDelay->setFont(font);

    font = groupTab->font();
    font.setPointSize(pointSize);
    groupTab->setFont(font);

    font = tableMode->font();
    font.setPointSize(pointSize);
    tableMode->setFont(font);
    resizeModeView();
}
