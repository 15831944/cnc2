#include <QTimer>
#include <QDebug>
#include <QProcess>

#include "form_home.h"
#include "aux_items.h"
#include "cnc.h"
#include "debug.h"

FormHome::FormHome(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    createButtons();

    QHBoxLayout *hboxApps = new QHBoxLayout;

    treeApps = new QTreeWidget;
    treeApps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    treeApps->setHeaderHidden(true);
    treeApps->setWordWrap(true);
    treeApps->setAutoFillBackground(true);
    treeApps->setColumnCount(1);

    itemWelcome = new QTreeWidgetItem(treeApps);
    itemWelcome->setText(0, tr("Welcome"));
    treeApps->addTopLevelItem(itemWelcome);

    itemSlicing = new QTreeWidgetItem(treeApps);
    itemSlicing->setText(0, tr("Slicing"));
    treeApps->addTopLevelItem(itemSlicing);

    itemComb = new QTreeWidgetItem(treeApps);
    itemComb->setText(0, tr("Comb"));
    treeApps->addTopLevelItem(itemComb);

#ifndef STONE
    itemUVSizing = new QTreeWidgetItem(treeApps);
    itemUVSizing->setText(0, tr("UV Sizing"));
    treeApps->addTopLevelItem(itemUVSizing);
#else
    itemUVSizing = nullptr;
#endif

    itemWelcome->setSelected(true);

    txtInfo = new QTextEdit;
    txtInfo->setEnabled(false);
    txtInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetSlicing = new SlicingWidget(par, this);
    widgetSlicing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetComb = new CombWidget(par, this);
    widgetComb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetUVSizing = new UVSizingWidget(par, this);
    widgetUVSizing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(txtInfo);
    stackedWidget->addWidget(widgetSlicing);
    stackedWidget->addWidget(widgetComb);
    stackedWidget->addWidget(widgetUVSizing);

    stackedWidget->setCurrentWidget(txtInfo);

    hboxApps->addWidget(treeApps);
    hboxApps->addWidget(stackedWidget);
    hboxApps->setStretch(0, 2);
    hboxApps->setStretch(1, 12);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hboxApps);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);

    //    treeBook->itemAt(0,0)->setBackgroundColor(0, Qt::blue);
    connect(treeApps, &QTreeWidget::itemClicked, this, &FormHome::onItemClicked);
    connect(treeApps, &QTreeWidget::itemActivated, this, &FormHome::onItemClicked);

    connect(widgetSlicing, &SlicingWidget::clicked, this, [&]() { emit contourPageClicked(); });
    connect(widgetComb, &CombWidget::clicked, this, [&]() { emit runPageClicked(false); });
    connect(widgetUVSizing, &UVSizingWidget::clicked, this, [&]() { emit editPageClicked(true); });
}

void FormHome::onItemClicked(QTreeWidgetItem *item, int) {
    if (item == itemWelcome) {
        stackedWidget->setCurrentWidget(txtInfo);
    }
    else if (item == itemSlicing) {
        stackedWidget->setCurrentWidget(widgetSlicing);
    }
    else if (item == itemComb) {
        stackedWidget->setCurrentWidget(widgetComb);
    }
    else if (itemUVSizing && item == itemUVSizing) {
        stackedWidget->setCurrentWidget(widgetUVSizing);
    }
}

FormHome::~FormHome() {}

void FormHome::createButtons() {
    btnHome = new QPushButton(tr("Home"));
    btnHome->setEnabled(false);

    btnContour = new QPushButton(tr("Contour"));
    btnContour->setStatusTip(tr("Load and edit a contour"));

    btnGCode = new QPushButton(tr("G-code"));
    btnGCode->setStatusTip(tr("Open G-code editor"));

    btnRun = new QPushButton(tr("Run"));
    btnRun->setStatusTip(tr("Open working panel"));

    btnSettings = new QPushButton(tr("Settings"));
    btnSettings->setStatusTip(tr("Change CNC settings"));

    btnPult = new QPushButton(tr("Pult"));
    btnPult->setStatusTip(tr("Manual ruling of the CNC"));

#ifndef STONE
    btnCenter = new QPushButton(tr("Center"));
#else
    btnCenter = new QPushButton;
    btnCenter->setEnabled(false);
#endif

    btnRec = new QPushButton(tr("Recovery"));
    btnRec->setStatusTip(tr("Recovery the last job"));

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

#if defined(DEV)
    btnTest = new QPushButton(tr("Test"));
#else
    btnTest = new QPushButton;
    btnTest->setEnabled(false);
#endif

    btnMinimize = new QPushButton(tr("Minimize"));
    btnMinimize->setStatusTip(tr("Minimize CNC program and open Linux desktop"));

    btnShutdown = new QPushButton(tr("Shutdown"));
    btnShutdown->setStatusTip(tr("Shutdown CNC"));
    btnShutdown->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));

    buttons = {
        btnHome, btnContour, btnGCode, btnRun, btnSettings, btnPult, btnCenter, btnRec,
        btn8, btn9, btnTest, btnMinimize, btnShutdown, btnHelp
    };

    gridButtons = new QGridLayout;

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnContour, 0, 1);
    gridButtons->addWidget(btnGCode, 0, 2);
    gridButtons->addWidget(btnRun, 0, 3);
    gridButtons->addWidget(btnSettings, 0, 4);
    gridButtons->addWidget(btnPult, 0, 5);

#ifndef STONE
    gridButtons->addWidget(btnCenter, 0, 6);
    gridButtons->addWidget(btnRec, 0, 7);
#else
    gridButtons->addWidget(btnRec, 0, 6);
    gridButtons->addWidget(btnCenter, 0, 7);
#endif

    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btnTest, 0, 10);
    gridButtons->addWidget(btnMinimize, 0, 11);
    gridButtons->addWidget(btnShutdown, 0, 12);
    gridButtons->addWidget(btnHelp, 0, 13);

    connect(btnContour, &QPushButton::clicked, this, [&]() { emit contourPageClicked(); });
    connect(btnGCode, &QPushButton::clicked, this, [&]() { emit editPageClicked(); });
    connect(btnRun, &QPushButton::clicked, this, [&]() { emit runPageClicked(false); });
    connect(btnSettings, &QPushButton::clicked, this, [&]() { emit settingsPageClicked(); });
    connect(btnPult, &QPushButton::clicked, this, [&]() { emit pultPageClicked(); });
    connect(btnCenter, &QPushButton::clicked, this, [&]() { emit centerPageClicked(); });
    connect(btnRec, &QPushButton::clicked, this, [&]() {
        par.loadBackup();

        if (!par.gcode.empty())
            emit runPageClicked(true);
    //    else
    //        ui->btnRec->setEnabled(false);
    });

    connect(btnTest, &QPushButton::clicked, this, [&]() { emit testPageClicked(); });
    connect(btnMinimize, &QPushButton::clicked, this, [&]() { emit programMinimize(); });
    connect(btnShutdown, &QPushButton::clicked, this, [&]() {
#ifdef LINUX
        QProcess::startDetached("shutdown -P now");
#else
//        QProcess::startDetached("shutdown -s -f -t 00");
        QProcess::startDetached("shutdown", {"-s", "-f", "-t", "00"});
#endif
    });
    connect(btnHelp, &QPushButton::clicked, this, [&]() { emit helpPageClicked(help_file); });
}

void FormHome::connectCnc(bool reset) {
    using namespace std;

    static int attempt_cnt = 1;
    string mcu_ver, fpga_ver;

    if (reset) {
        attempt_cnt = 1;
    }
    par.cncConnected = false;

    showConnecting(attempt_cnt);
    qDebug() << "Connecting. Attempt " + QString::number(attempt_cnt) + "...";

    if (!par.cnc.isOpen()) {
        par.cnc.open();

        if (!par.cnc.isOpen()) {
            if (attempt_cnt++ < CONNECTION_ATTEMPTS) {
                QTimer::singleShot(5000, [&](){
                    this->connectCnc(false);
                });
            } else {
                showComNotFound();
            }
            return;
        }
    }

    try {
        bool OK = par.cnc.reset();
        if (OK) {
            mcu_ver = par.cnc.readVersion();
            fpga_ver = par.cnc.m_fpga.readVersion();
            par.cncConnected = true;
        }
//        par.cnc.writeInputLevel(CncParam::inputLevel); // - in reset
     } catch (string& s) {
        qDebug("%s\n", s.c_str());
    } catch (exception& e) {
        qDebug("Exception: %s\n", e.what());
    } catch (...) {
        qDebug("Unknown exception\n");
    }

    if (!par.cncConnected) {
        if (attempt_cnt++ < CONNECTION_ATTEMPTS) {
            QTimer::singleShot(5000, [&](){
                this->connectCnc(false);
            });
        } else {
            showNoConnection();
        }
    }
    else
        showVersion(mcu_ver, fpga_ver);
}

void FormHome::showConnecting(int attempt) {
    QString s = R"(<p><b>)" + tr("Info") + ": " + R"(</b>)" + tr("Connecting.") + " " + "Attempt" + " " + QString::number(attempt) + "..." + R"(</p>)";
    QString info = info_header + R"(<p style="text-align:center;"><h3>)" + s + R"(</h3></p>)";
    txtInfo->setHtml(info);
}

void FormHome::showComNotFound() {
    QString s = R"(<p><font color=red>)" + tr("Error") + ": " + R"(</font><i>)" + tr("Serial port is not found") + R"(</i></p>)";
    QString info = info_header + R"(<p style="text-align:right;">)" + s + R"(</p>)";
    txtInfo->setHtml(info);
}

void FormHome::showNoConnection() {
    QString s = R"(<p><font color=red>)" + tr("Error") + ": " + R"(</font><i>)" + tr("No connection. Check conection between computer and CNC board") + R"(</i></p>)";
    QString info = info_header + R"(<p style="text-align:right;">)" + s + R"(</p>)";
    txtInfo->setHtml(info);
}

void FormHome::showVersion(const std::string &mcu, const std::string &fpga) {
    QString info = info_header +
            R"(<font color=black><p style="text-align:center;"><h3>)" + QString(mcu.c_str()) + R"(</h3></p></font>)" +
            R"(<font color=black><p style="text-align:center;"><h3>)" + QString(fpga.c_str()) + R"(</h3></p></font>)";
    txtInfo->setHtml(info);
}

void FormHome::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons) {
        QFont font = b->font();
        font.setPointSize(pointSize);
        b->setFont(font);
//        b->setStyleSheet("font: bold");
    }

    QFont font = treeApps->font();
    font.setPointSize(pointSize);
    treeApps->setFont(font);

    itemWelcome->setFont(0, font);
    itemSlicing->setFont(0, font);
    itemComb->setFont(0, font);
    if (itemUVSizing) itemUVSizing->setFont(0, font);

    widgetSlicing->setFontPointSize(pointSize);
    widgetComb->setFontPointSize(pointSize);
    widgetUVSizing->setFontPointSize(pointSize);
}
