#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QTranslator>

#include "program_param.h"

//#include <QDBusConnection>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator translator;
    {
        ProgramParam::loadParam();

        switch (ProgramParam::lang) {
        case InterfaceLanguage::RUSSIAN:
            translator.load("icnc_ru", ".");
            break;
        default: break;
        }
    }
    qApp->installTranslator(&translator);
    qDebug("Hello world!");

    MainWindow w;
    QIcon icon("icnc.png");
//    w.setWindowIcon(icon);
//    w.setWindowIconText(QObject::tr("Meatec CNC"));
    app.setWindowIcon(icon);
    app.setApplicationDisplayName(QObject::tr("Meatec CNC"));
//    app.setDesktopFileName(QObject::tr("Meatec CNC (Run)"));

//    int fontSize = ProgramParam::loadFontSize();

    w.show();

//    QTimer::singleShot(0, &w, &MainWindow::onLoad);

    QThread::currentThread()->setPriority(QThread::Priority::HighPriority);

//    QDBusConnection bus = QDBusConnection::sessionBus();
//    if(bus.isConnected())
//    {
//        QString services[MAX_SERVICES] = {
//            "org.freedesktop.ScreenSaver",
//            "org.gnome.SessionManager"
//        };
//        QString paths[MAX_SERVICES] = {
//            "/org/freedesktop/ScreenSaver",
//            "/org/gnome/SessionManager"
//        };

//        for(int i = 0; i < MAX_SERVICES ; i++)
//        {
//            QDBusInterface screenSaverInterface(
//                services[i], paths[i],services[i], bus, this);
//            if (!screenSaverInterface.isValid())
//                continue;

//            QDBusReply<uint> reply = screenSaverInterface.call(
//                "Inhibit", "YOUR_APP_NAME", "REASON");
//            if (reply.isValid())
//            {
//                cookieID = reply.value();
//                qDebug()<<"succesful"
//            } else {
//                QDBusError error =reply.error();
//                qDebug()<<error.message()<<error.name();
//            }
//        }
//    }

    return app.exec();
}
