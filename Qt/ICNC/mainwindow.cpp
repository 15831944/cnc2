#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QScreen>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QTranslator>
#include <QLocale>

#include "main.h"

int MainWindow::cnt = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_formCenter(nullptr),
    m_helpWidget(nullptr),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(FULL_SCREEN)
    setWindowState(Qt::WindowFullScreen);
#else
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
//    this->setMaximumSize(QSize(geometry.width(), geometry.height()));
    this->resize(geometry.width() / 2, geometry.height() / 2);
//    this->setWindowState(Qt::WindowFullScreen);
#endif

    m_formHome = new FormHome(par, this);
    m_formContour = new FormContour(par, this);
    m_formEdit = new FormEdit(par, this);
    m_formRun = new FormRun(par, this);
    m_formSettings = new FormSettings(par, this);
    m_formPult = new FormPult(par, this);
#ifndef STONE
    m_formCenter = new FormCenter(par, this);
#endif
    m_formTest = new FormTest(par, this);

#if defined(STONE)
    m_formPasses = new FormPassesStone(par, this);
#else
    m_formPasses = new FormPasses(par, this);
#endif

    m_formHelp = new FormHelp(par, this);

    connect(m_formContour, &FormContour::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formEdit, &FormEdit::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formRun, &FormRun::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formSettings, &FormSettings::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formPult, &FormPult::homePageClicked, this, &MainWindow::onHomePageClicked);
    if (m_formCenter) connect(m_formCenter, &FormCenter::homePageClicked, this, &MainWindow::onHomePageClicked);
    connect(m_formTest, &FormTest::homePageClicked, this, &MainWindow::onHomePageClicked);

    connect(m_formHome, &FormHome::contourPageClicked, this, &MainWindow::onContourPageClicked);
    connect(m_formEdit, &FormEdit::contourPageClicked, this, [=]() {
        m_formContour->resetFileOpen();
        onContourPageClicked();
    });

#if defined(STONE)
    connect(m_formPasses, &FormPassesStone::backPageClicked, this, &MainWindow::onContourPageClicked);
#else
    connect(m_formPasses, &FormPasses::backPageClicked, this, &MainWindow::onContourPageClicked);
#endif

    connect(m_formHome, &FormHome::editPageClicked, this, [=](bool auto_plot) { onEditPageClicked(auto_plot); });
    connect(m_formTest, &FormTest::editPageClicked, this, [=]() { onEditPageClicked(false); });
    connect(m_formContour, &FormContour::editPageClicked, this, [=](bool auto_plot) {
        onEditPageClicked(auto_plot);
    });

    connect(m_formEdit, &FormEdit::runPageClicked, this, [=]() { onRunPageClicked(false); });
    connect(m_formHome, &FormHome::runPageClicked, this, [=](bool backup) { onRunPageClicked(backup); });

    connect(m_formHome, &FormHome::settingsPageClicked, this, &MainWindow::onSettingsPageClicked);
    connect(m_formHome, &FormHome::pultPageClicked, this, &MainWindow::onPultPageClicked);
    connect(m_formHome, &FormHome::centerPageClicked, this, &MainWindow::onCenterPageClicked);

    connect(m_formHome, &FormHome::testPageClicked, this, &MainWindow::onTestPageClicked);
    connect(m_formEdit, &FormEdit::testPageClicked, this, &MainWindow::onTestPageClicked);

    connect(m_formContour, &FormContour::passesPageClicked, this, &MainWindow::onPassesPageClicked);

    connect(m_formHome, &FormHome::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formHome, file_name); });
    connect(m_formContour, &FormContour::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formContour, file_name); });

#if defined(STONE)
    connect(m_formPasses, &FormPassesStone::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPasses, file_name); });
#else
    connect(m_formPasses, &FormPasses::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPasses, file_name); });    
#endif

    connect(m_formEdit, &FormEdit::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formEdit, file_name); });
    connect(m_formRun, &FormRun::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formRun, file_name); });
    connect(m_formSettings, &FormSettings::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formSettings, file_name); });
    connect(m_formPult, &FormPult::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formPult, file_name); });

    if (m_formCenter) connect(m_formCenter, &FormCenter::helpPageClicked, this, [&](const QString& file_name) { onHelpPageClicked(m_formCenter, file_name); });

    connect(m_formHelp, &FormHelp::backClicked, this, &MainWindow::onBackHelpClicked);

    connect(m_formSettings, &FormSettings::showInfo, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: green; font: " + QString::number(par.fontSize) + "px;");
        // todo: html color
        statusBar()->showMessage(text);
    });

    connect(m_formSettings, &FormSettings::showWarning, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: blue; font: " + QString::number(par.fontSize) + "px;");
        statusBar()->showMessage(tr("Warning") + ": " + text);
    });

    connect(m_formSettings, &FormSettings::showError, this, [&](const QString& text) {
//        ui->statusBar->setStyleSheet("color: red; font: bold " + QString::number(par.fontSize) + "px;");
        statusBar()->showMessage(tr("Error") + ": " + text);
    });

    connect(m_formHome, &FormHome::programMinimize, this, [&]() {
        setWindowState(Qt::WindowMinimized);
    });

    connect(m_formContour, &FormContour::fileNameChanged, this, &MainWindow::onFileNameChanged);
    connect(m_formEdit, &FormEdit::fileNameChanged, this, &MainWindow::onFileNameChanged);

    updateFontPointSize();

    m_stackedWidget = new QStackedWidget;
    if (m_stackedWidget) {
        m_stackedWidget->addWidget(m_formHome);
        m_stackedWidget->addWidget(m_formContour);
        m_stackedWidget->addWidget(m_formEdit);
        m_stackedWidget->addWidget(m_formRun);
        m_stackedWidget->addWidget(m_formSettings);
        m_stackedWidget->addWidget(m_formPult);
        if (m_formCenter) m_stackedWidget->addWidget(m_formCenter);
        m_stackedWidget->addWidget(m_formTest);

        m_stackedWidget->addWidget(m_formPasses);
        m_stackedWidget->addWidget(m_formHelp);
    }

//    QVBoxLayout *layout = new QVBoxLayout;
//    layout->addWidget(stackedWidget);
//    setLayout(layout);

    this->setCentralWidget(m_stackedWidget);

//    for (auto s: QStyleFactory::keys()) {
//        qDebug() << s;
//    }
//    QApplication::setStyle(QStyleFactory::create(QStyleFactory::keys()[1]));
    labelMeatec = new QLabel(R"(<font color=white><b><h1>)" + tr("Meatec") + R"(</h1></b></font>)");
    labelTitle = new QLabel("File name");
    labelTime = new QLabel;
    showTime();
//    lcdTime = new QLCDNumber();
//    lcdTime->setSegmentStyle(QLCDNumber::SegmentStyle::Filled);

    QWidget* spacerWidget1 = new QWidget;
    if (spacerWidget1) {
        spacerWidget1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidget1->setVisible(true);
    }

    QWidget* spacerWidget2 = new QWidget;
    if (spacerWidget2) {
        spacerWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidget2->setVisible(true);
    }

    ui->mainToolBar->setStyleSheet("background-color:orange;");
    ui->mainToolBar->addWidget(labelMeatec);
    ui->mainToolBar->addWidget(spacerWidget1);
    ui->mainToolBar->addWidget(labelTitle);
    ui->mainToolBar->addWidget(spacerWidget2);
    ui->mainToolBar->addWidget(labelTime);
//    ui->mainToolBar->addWidget(lcdTime);

    QTimer *timer = new QTimer(this);
    if (timer) {
        connect(timer, &QTimer::timeout, this, &MainWindow::showTime);
        timer->start(1000);
    }

//    statusBar()->showMessage("");

    onHomePageClicked();
}

void MainWindow::showTime() {
    QDateTime dt = QDateTime();
    dt = QDateTime::currentDateTime();

    QLocale loc = QLocale();
    QString s = R"(<h3>)" + loc.toString(dt, QLocale::ShortFormat) + R"(<\h3>)";

//    s = R"(<h3>)" + time.toString("h:mm") + " " + date.toString("d/M/yyyy") + R"(<\h3>)";

//    if (time.second() & 1) {
//        str[2] = ' ';
//    }
    if (labelTime) labelTime->setText(s);
}

void MainWindow::updateFontPointSize() {
    if (m_formHome)     m_formHome->setFontPointSize(par.fontSize);
    if (m_formContour)  m_formContour->setFontPointSize(par.fontSize);
    if (m_formEdit)     m_formEdit->setFontPointSize(par.fontSize);
    if (m_formRun)      m_formRun->setFontPointSize(par.fontSize);
    if (m_formSettings) m_formSettings->setFontPointSize(par.fontSize);
    if (m_formPult)     m_formPult->setFontPointSize(par.fontSize);
    if (m_formCenter)   m_formCenter->setFontPointSize(par.fontSize);
//    m_formTest->setFontPointSize(par.fontSize);
    if (m_formPasses)   m_formPasses->setFontPointSize(par.fontSize);
    if (m_formHelp)     m_formHelp->setFontPointSize(par.fontSize);

    if (ui) {
        QFont font = ui->statusBar->font();
        font.setPointSize(par.fontSize);
        ui->statusBar->setFont(font);
//      ui->statusBar->setStyleSheet("color: black; font: " + QString::number(par.fontSize) + "px;");
    }
}

//void MainWindow::onLoad() { m_formHome->connectCnc(); }

MainWindow::~MainWindow() {
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void MainWindow::onFileNameChanged(const QString& file_name) {
    labelTitle->setText(R"(<font color=black><h2>)" + file_name + R"(</h2></font>)");
}

void MainWindow::onContourPageClicked() {
    if (!m_stackedWidget || !m_formContour) return;

    onFileNameChanged(m_formContour->getFileName());

    this->setWindowTitle(tr("Load DXF contours"));
    m_stackedWidget->setCurrentWidget(m_formContour);
    m_formContour->init();
}

void MainWindow::onEditPageClicked(bool auto_plot) {
    if (!m_stackedWidget || !m_formEdit) return;

    this->setWindowTitle(tr("Code Editor"));
    onFileNameChanged(m_formEdit->getFileName());
    m_stackedWidget->setCurrentWidget(m_formEdit);
    m_formEdit->init(auto_plot);
}

void MainWindow::onRunPageClicked(bool recovery) {
    if (!m_stackedWidget || !m_formRun) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Job Panel"));
    m_stackedWidget->setCurrentWidget(m_formRun);
    m_formRun->init(recovery);
}

void MainWindow::onSettingsPageClicked() {
    if (!m_stackedWidget || !m_formSettings) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("CNC Settigns"));
    m_stackedWidget->setCurrentWidget(m_formSettings);
}

void MainWindow::onPultPageClicked() {
    if (!m_stackedWidget || !m_formPult) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("CNC Pult"));
    m_stackedWidget->setCurrentWidget(m_formPult);
    m_formPult->init();
}

void MainWindow::onCenterPageClicked() {
    if (!m_stackedWidget || !m_formCenter) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Centering"));
    m_stackedWidget->setCurrentWidget(m_formCenter);
    m_formCenter->init();
}

void MainWindow::onTestPageClicked() {
    if (!m_stackedWidget || !m_formTest) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Test"));
    m_stackedWidget->setCurrentWidget(m_formTest);
}

//
void MainWindow::onHomePageClicked() {
    if (!m_stackedWidget || !m_formHome) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Home Panel"));
    m_stackedWidget->setCurrentWidget(m_formHome);    
}

void MainWindow::onPassesPageClicked() {
    if (!m_stackedWidget || !m_formPasses) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Cutting settings"));
    m_stackedWidget->setCurrentWidget(m_formPasses);
}

void MainWindow::onHelpPageClicked(QWidget *source, const QString& helpHtmlFileName) {
    if (!m_stackedWidget || !m_formHelp || !source || helpHtmlFileName.isEmpty()) return;

    labelTitle->setText("");

    this->setWindowTitle(tr("Help"));
    m_helpWidget = source;
    m_stackedWidget->setCurrentWidget(m_formHelp);
    m_formHelp->init(helpHtmlFileName);
}

void MainWindow::onBackHelpClicked() {
    if (m_stackedWidget && m_helpWidget) {
        m_stackedWidget->setCurrentWidget(m_helpWidget);
        m_helpWidget = nullptr;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key::Key_F11) {
        setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    setWindowState(Qt::WindowFullScreen);

    if (cnt == 0) {
        cnt++;
        if (m_formHome) m_formHome->connectCnc();
#if defined(STONE)
        par.cnc.writeSemaphoreCncEnable(true);
#else
        par.cnc.writeCncEnable(true);
#endif
    }
    if (event) event->ignore();
}
