#include "form_run.h"
#include <QDebug>
#include <QTimer>

using namespace std;

FormRun::FormRun(ProgramParam& par, QWidget *parent) :
    QWidget(parent), par(par), cncReaderEna(false), adcEnable(false), currentCursorPosition(0), cutStateAbortReq(false), m_speed(WireSpeed()), remain_tmr(0)
{
    runWidget = new RunWidget(this);
    createSpinBoxes(); // row 0
    addButtons(); // row 1

//    setFontPointSize(14);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(runWidget);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    timer = new StartStopElapsedTimer;

    connect(runWidget->txtCode, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    highlightCurrentLine();

    m_report.bind(runWidget->txtMsg);

    connect(runWidget->btnStart, &QPushButton::clicked, this, &FormRun::on_btnStart_clicked);
    connect(runWidget->btnReverse, &QPushButton::clicked, this, &FormRun::on_btnReverse_clicked);
    connect(runWidget->btnCancel, &QPushButton::clicked, this, &FormRun::on_btnCancel_clicked);
    connect(runWidget->checkIdle, &QCheckBox::clicked, this, [&](bool checked) {
        par.appState.setIdleRun(checked);
    });

//    bindNumberButtons(numWidth, btnWidthInc, btnWidthDec);

//    connect(sender, &Sender::valueChanged,
//        [=]( const QString &newValue ) { receiver->updateValue( "senderValue", newValue ); }
//    );
}

FormRun::~FormRun() {
    stopCncReader();
    if (timer) {
        delete timer;
        timer = nullptr;
    }
}

void FormRun::setFontPointSize(int pointSize) {
    runWidget->setFontPointSize(pointSize);

    for (QPushButton* b: buttons) {
        if (!b) continue;

        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }

    for (QSpinBox* const numBox: numBoxes) {
        if (!numBox) continue;

//        numBox->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = numBox->font();
        font.setPointSize(pointSize);
//        font.setBold(true);
        numBox->setFont(font);
    }

    vector<QLabel*> labels = {labelDrum, labelWidth, labelRatio, labelVoltage, labelCurrent};
    for (QLabel* const label: labels) {
        if (!label) continue;

//        numBox->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        QFont font = label->font();
        font.setPointSize(pointSize);
//        font.setBold(true);
        label->setFont(font);
    }
}

void FormRun::addButtons() {
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
    btnVoltage->setCheckable(true);
#ifndef STONE
    btnVoltage->setText(tr("Voltage"));
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

    buttons = {btnBreak, btnPump, btnDrum, btnDrumVelDec, btnDrumVelInc,\
               btnVoltage, btnVoltageDec, btnVoltageInc, btnCurrentDec, btnCurrentInc, btnWidthDec, btnWidthInc, btnRatioInc, btnRatioDec,\
              btnHome, btnHelp};

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

    gridButtons->addWidget(btnBreak , 1, 0);
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

    connect(btnHome, &QPushButton::clicked, this, &FormRun::on_btnHome_clicked);
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

    connect(runWidget->btnHold, &QPushButton::clicked, [&]() {
        try {
            par.cnc.writeHoldEnable( runWidget->btnHold->isChecked() );
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

void FormRun::createSpinBoxes() {
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

    numBoxes = {numDrumVel, numWidth, numRatio, numVoltage, numCurrent};

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
    runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());
    runWidget->numSpeed->setValue(m_speed.get());
    runWidget->numSpeed->setDecimals(1);

    connect(runWidget->numSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double x) {
        m_speed.set(x);
        par.cnc.writeSpeed(m_speed);
    });

//    connect(runWidget->btnSpeed, &QPushButton::clicked, this, [&]() {
//        m_speed.set(runWidget->numSpeed->value());
//        par.cnc.writeSpeed(m_speed);
//    });

    connect(runWidget->speedMMM, &QRadioButton::clicked, this, [&]() {
        if (m_speed.mode() != WireSpeed::Mode::MMM) {
            m_speed.changeMode(WireSpeed::Mode::MMM);
            runWidget->numSpeed->blockSignals(true);

            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            runWidget->numSpeed->setSingleStep(0.1);
            runWidget->numSpeed->setDecimals(1);
            runWidget->numSpeed->setValue(m_speed.get());
            runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            runWidget->numSpeed->blockSignals(false);
        }
    });
    connect(runWidget->speedUMS, &QRadioButton::clicked, this, [&]() {
        if (m_speed.mode() != WireSpeed::Mode::UMS) {
            m_speed.changeMode(WireSpeed::Mode::UMS);
            runWidget->numSpeed->blockSignals(true);

            runWidget->numSpeed->setRange(DBL_MIN, DBL_MAX);
            runWidget->numSpeed->setSingleStep(1);
            runWidget->numSpeed->setDecimals(0);
            runWidget->numSpeed->setValue(m_speed.get());
            runWidget->numSpeed->setRange(m_speed.min(), m_speed.max());

            runWidget->numSpeed->blockSignals(false);
        }
    });
}

bool FormRun::loadGcode() {
    list<string> gframes = par.gcode.toFrameList();
    par.cnc.write(gframes);
    list<string> read_gframes = par.cnc.read();

    string error_text;
    bool OK = GCode::CompareFrames(gframes, read_gframes, error_text);

    if (!OK) {
        runWidget->txtMsg->append(QString(error_text.c_str()));
        return false;
    }

    string txt = GCode::toText(read_gframes);
    qDebug() << txt.c_str();
    return true;
}

void FormRun::init(bool recovery) {
    m_report.clear();
    par.cnc.bindReporter(&m_report);

    _init();

    if (recovery) {
        bool mcu_backup_valid = false; // for one print debug

        // Local backup has high priority
        if (!par.cncContext.valid()) {
            cnc_context_t mcu_ctx = par.cnc.readBackup();
            toDebug(&mcu_ctx);
            mcu_backup_valid = mcu_ctx.field.backup_valid;

            if (mcu_ctx.field.backup_valid)
                par.cncContext.set(mcu_ctx);
        }

        if (par.cncContext.valid()) {
            try {
                par.cnc.reset(); // includes Input Levels                
#if defined(STONE)
                par.cnc.writeSemaphoreCncEnable(true);
#else
                par.cnc.writeCncEnable(true);
#endif
                bool OK = loadGcode();
                if (!OK) return;

                if (!mcu_backup_valid)
                    toDebug(&par.cncContext.get());

                par.cnc.initialContext(par.cncContext.get());

                par.appState.recovery(); // pause
                updateButtons();

                par.cnc.imitEna(false);
                if (timer) {
                    timer->clear();
                    timer->start();
                }
            } catch (string& s) {
                m_report.write(s.c_str());
                return;
            } catch (exception& e) {
                m_report.write("Error: exception (" + QString(e.what()) + ")");
                return;
            } catch (...) {
                m_report.write("Error: Form run unknown exception\n");
                return;
            }
        }
    }

    if (!cncReaderEna)
        startCncReader();

#ifndef STONE
    startAdc();
#endif
}

void FormRun::_init() {
    par.gcodeCnc.clear();
    runWidget->txtCode->clear();
    par.workContours.clear();
    par.mapGcodeToContours.clear();
    runWidget->txtMsg->clear();

    if (par.gcode.empty())
        runWidget->txtMsg->setText(tr("Error") + ": " + tr("No G-code") + "\n");
    else {
        par.gcode.normalize();

        if (!par.gcode.empty()) {
            par.gcodeCnc = par.gcode.toText().c_str();
//            text = addLineNum(text);
            runWidget->txtCode->setPlainText(par.gcodeCnc);
            setCursorToBegin();
            par.workContours = ContourList( par.gcode.getContours(&par.mapGcodeToContours) );
        }
        else
            runWidget->txtMsg->insertPlainText(tr("Error") + ": " + tr("G-code error") + "\n");
    }

    runWidget->checkIdle->setCheckState( par.appState.idleRun() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    updateButtons();
    runWidget->setButtonsEnabled(0, 0, 0, 1);

    runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);

    if (!par.cnc.isOpen())
        runWidget->txtMsg->insertPlainText(tr("Error") + ": " + tr("No CNC connection") + "\n");
}

void FormRun::highlightCurrentLine() {
    setCursor(currentCursorPosition);

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = runWidget->txtCode->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    runWidget->txtCode->setExtraSelections(extraSelections);
}

void FormRun::on_btnHome_clicked() {
    stopCncReader();
    stopAdc();
    emit homePageClicked();
}

QString FormRun::addLineNum(QString txt) {
    QStringList list = txt.split(QRegExp("(\\n|\\r\\n|\\n\\r)"));
    QString res;
    int i = 0;
    for (auto it = list.begin(); it != list.end(); ++it, i++)
        res += QString("N%1 %2\n").arg(QString::number(i), *it);

    return res;
}

void FormRun::updateButtons() {
    switch (par.appState.state()) {
    case AppState::STATES::ST_NONE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {tr("Start cutting"), tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(true);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnDrum->isChecked()))
            runWidget->setButtonsEnabled(1,0,0,1);
        else
            runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_IDLE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {tr("Start cutting"), tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(true);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnDrum->isChecked()))
            runWidget->setButtonsEnabled(1,0,0,1);
        else
            runWidget->setButtonsEnabled(0,0,0,1);

        break;
    case AppState::STATES::ST_RUN:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {tr("Make a pause"), "", ""});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_PAUSE:
        runWidget->setButtonsText({tr("Start"), tr("Back"), tr("Cancel")},
                                  {tr("Start cutting"), tr("Go to back"), tr("Cancel program")});
        btnHome->setEnabled(false);

        if (par.appState.idleRun() || (btnBreak->isChecked() && btnPump->isChecked() && btnDrum->isChecked()))
            runWidget->setButtonsEnabled(1,1,1,1);
        else
            runWidget->setButtonsEnabled(0,1,1,1);

        break;
    case AppState::STATES::ST_REV:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {tr("Make a pause"), "", ""});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_CANCEL:
        runWidget->setButtonsText({"", tr("Shortcut"), "Reset"},
                                  {"", tr("Fast return to start position"), tr("Reset CNC")});
        runWidget->setButtonsEnabled(0,1,1,1);
        btnHome->setEnabled(false);
        break;
    case AppState::STATES::ST_SHORT_REV:
        runWidget->setButtonsText({tr("Stop"), "", ""},
                                  {tr("Make a pause"), tr(""), tr("")});
        runWidget->setButtonsEnabled(1,0,0,0);
        btnHome->setEnabled(false);
        break;
    default:
        runWidget->setButtonsText({"", "", tr("Reset")},
                                  {"", "", tr("Reset CNC")});
        runWidget->setButtonsEnabled(0,0,1,1);
        btnHome->setEnabled(false);
        break;
    }
}

void FormRun::on_btnStart_clicked() {
    switch (par.appState.state()) {
    case AppState::STATES::ST_NONE:
        _init();
        [[clang::fallthrough]];

    case AppState::STATES::ST_IDLE:
        try {
            par.cnc.clearBackup();
            par.saveGcode();
            bool OK = loadGcode();

            if (OK) {
                par.cnc.imitEna(false);

                if (runWidget->checkIdle->isChecked() || (btnBreak->isChecked() && btnPump->isChecked() && btnDrum->isChecked())) {
                    if (!runWidget->checkIdle->isChecked())
                        par.cnc.writeHoldEnable(true);

                    par.cnc.runReq();
                    if (timer) {
                        timer->clear();
                        timer->start();
                    }
                }
            }
        } catch (string& s) {
            m_report.write(s.c_str());
            return;
        } catch (exception& e) {
            m_report.write("Error: exception (" + QString(e.what()) + ")");
            return;
        } catch (...) {
            m_report.write("Error: Form run unknown exception\n");
            return;
        }
        break;

    case AppState::STATES::ST_RUN:
    case AppState::STATES::ST_REV:
        par.cnc.stopReq();
        if (timer) timer->stop();
        break;

    case AppState::STATES::ST_PAUSE:
        if (runWidget->checkIdle->isChecked() || (btnBreak->isChecked() && btnPump->isChecked() && btnDrum->isChecked())) {
            if (!runWidget->checkIdle->isChecked())
                par.cnc.writeHoldEnable(true);

            par.cnc.runReq();
            if (timer) timer->start();
        }
        break;

    default:
//        par.cnc.stopReq();
        break;
    }

    // todo: more reactions
    par.appState.next(AppState::BUTTON::SIG_START);
    updateButtons();
}

void FormRun::on_btnReverse_clicked() {
    if (!runWidget->checkIdle->isChecked())
        par.cnc.writeHoldEnable(true);

    par.cnc.revReq();
    if (timer) timer->start();
    par.appState.next(AppState::BUTTON::SIG_REVERSE);
    updateButtons();
}

void FormRun::on_btnCancel_clicked() {
    par.saveCncContext();
    int drum_vel = numDrumVel->value();

    par.cnc.cancelReq();
    par.cnc.writeDrumVel(drum_vel);

    par.appState.next(AppState::BUTTON::SIG_CANCEL);
    updateButtons();
}

void FormRun::setCursor(int row) {
    currentCursorPosition = row;

    QTextCursor textCursor = runWidget->txtCode->textCursor();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, currentCursorPosition);

    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::setCursorToBegin() {
    QTextCursor textCursor = runWidget->txtCode->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::setCursorToEnd() {
    QTextCursor textCursor = runWidget->txtCode->textCursor();

    textCursor.movePosition(QTextCursor::End);

    currentCursorPosition = textCursor.position();

    runWidget->txtCode->setTextCursor(textCursor);
    runWidget->txtCode->update();
}

void FormRun::startCncReader() {
    if (par.cnc.isOpen()) {
        qDebug()<<"CNC reader: Start";
        cncReaderEna = true;
        remain_tmr = 0;
        readCncContext();
    }
}

void FormRun::stopCncReader() {
    qDebug()<<"CNC reader: Stop";
    cncReaderEna = false;
    remain_tmr = 0;
}

void FormRun::readCncContext() {
    if (cncReaderEna) {
//        qDebug()<<"CNC reader: Reading...";

        try {
            par.cncContext = par.cnc.readCncContext();
            const CncContext& ctx = par.cncContext;
#ifndef STONE
            runWidget->txtMsg->setText(QString( ctx.toStringRunDebug().c_str() ));
#endif
//            qDebug() << "CNC reader: " + QString(ctx.toString().c_str());

            runWidget->setPositionLabels(0, ctx.x());
            runWidget->setPositionLabels(1, ctx.y());
            runWidget->setPositionLabels(2, ctx.u());
            runWidget->setPositionLabels(3, ctx.v());

            runWidget->setEncoderLabels(0, ctx.encoderX());
            runWidget->setEncoderLabels(1, ctx.encoderY());

            qint64 ms = timer ? timer->elapsed() : 0;
            runWidget->setElapsedTime(ms);

            runWidget->setLimitSwitches(ctx.limitSwitches());

            btnDrum->blockSignals(true);
            btnPump->blockSignals(true);
            btnBreak->blockSignals(true);
            btnVoltage->blockSignals(true);
            numDrumVel->blockSignals(true);
            numWidth->blockSignals(true);
            numRatio->blockSignals(true);
            numVoltage->blockSignals(true);
            numCurrent->blockSignals(true);

            runWidget->btnHold->blockSignals(true);
            runWidget->numSpeed->blockSignals(true);

            btnDrum->setChecked(ctx.isDrumEnable());
            btnPump->setChecked(ctx.pumpEnabled());
            btnBreak->setChecked(ctx.wireControlEnabled());
            btnVoltage->setChecked(ctx.voltageEnabled());

            runWidget->btnHold->setChecked(ctx.hold());

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
            else if (static_cast<int>(ctx.get().field.voltage_level) > numVoltage->maximum()) {
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

            m_speed.setMMM(ctx.speed());

            if (m_speed.get() < m_speed.min()) {
                runWidget->numSpeed->setValue(m_speed.min());
                qDebug("Error: MIN CNC Speed: %f", ctx.speed());
            }
            else if (m_speed.get() > m_speed.max()) {
                runWidget->numSpeed->setValue(m_speed.max());
                qDebug("Error: MAX CNC Speed: %f", ctx.speed());
            }
            else
                runWidget->numSpeed->setValue(m_speed.get());

            btnDrum->blockSignals(false);
            btnPump->blockSignals(false);
            btnBreak->blockSignals(false);
            btnVoltage->blockSignals(false);
            numDrumVel->blockSignals(false);
            numWidth->blockSignals(false);
            numRatio->blockSignals(false);
            numVoltage->blockSignals(false);
            numCurrent->blockSignals(false);

            runWidget->btnHold->blockSignals(false);
            runWidget->numSpeed->blockSignals(false);

            if (par.appState.isWork() && ctx.isWork()) {
                setCursor(ctx.frameNum());
                onFrameChanged(ctx.frameNum(),
                               fpoint_t(ctx.x() / X_SCALE, ctx.y() / Y_SCALE),
                               fpoint_t(ctx.u() / U_SCALE, ctx.v() /  V_SCALE)
                               );

                if (remain_tmr >= REMAIN_TIMER_MAX) {
                    remain_tmr = 0;
                    double length = par.workContours.botLength();
                    double speed = length / ms;

                    double full_length = par.workContours.botLengthFull();

                    double remain_length = full_length - length;
                    double remain_time = remain_length / speed;

                    if (remain_time < 0) remain_time = 0;
                    runWidget->setRemainTime(static_cast<qint64>(remain_time));
                }
                else
                    ++remain_tmr;
            }

            if (par.appState.isError() || ctx.isError() || cutStateAbortReq) {
                if (ctx.isError()) {
                    par.cnc.cancelReq();
                    par.appState.update(ctx);
                    updateButtons();
                    qDebug("CutState: Error");
                }
                else
                    par.cnc.stopReq();

                if (par.appState.isError())
                    qDebug("RunWidgetState: Error");

                if (cutStateAbortReq) {
                    qDebug("CutState: Abort request");
                    cutStateAbortReq = false;
                }

                if (timer) timer->stop();
            }
            else if (par.appState.isWork() && !ctx.isWork()) {
                par.appState.reset();
                updateButtons();
                setCursorToEnd();
                if (timer) timer->stop();
            }
            else {
                par.appState.update(ctx);
                updateButtons();
            }
        }
        catch (...) {

        }

        QTimer::singleShot(POLLING_TIME, this, &FormRun::readCncContext);
    }
}

void FormRun::startAdc() { // on init
    if (par.cnc.isOpen()) {
        qDebug()<<"ADC: Start";
        adcEnable = true;
        readAdc();
    }
    else
        adcEnable = false;
}

void FormRun::stopAdc() { // at to home
    qDebug()<<"ADC: Stop";
    adcEnable = false;
}

void FormRun::readAdc() {
    if (adcEnable) {
        cnc_adc_volt_t adc = par.cnc.readADCVolt();
        runWidget->setAdc(adc);
        QTimer::singleShot(ADC_POLLING_TIME, this, &FormRun::readAdc);
    }
}

void FormRun::onFrameChanged(int frame_num, const fpoint_t& bot_pos, const fpoint_t& top_pos) {
    if (!par.workContours.empty() && !par.mapGcodeToContours.empty()) {
        if (frame_num >= 0) {
            par.workContours.select( par.getDxfEntityNum(size_t(frame_num)) );

            if (par.gcodeSettings.isUV()) {
                par.workContours.setBotPos(bot_pos);
                par.workContours.setTopPos(bot_pos + top_pos);
            }
            else
                par.workContours.setBotPos(bot_pos);
        }
        else
            par.workContours.clearSelected();

        runWidget->plot(par.workContours, par.swapXY, par.reverseX, par.reverseY, par.showXY);
    }
}
