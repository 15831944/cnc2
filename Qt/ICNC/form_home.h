#ifndef FORMHOME_H
#define FORMHOME_H

#include <vector>

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>

#include "program_param.h"
#include "slicing_widget.h"
#include "comb_widget.h"
#include "uv_sizing_widget.h"
#include "main.h"

class FormHome : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QTreeWidget *treeApps;
    QStackedWidget* stackedWidget;
    QTextEdit* txtInfo;
    SlicingWidget* widgetSlicing;
    CombWidget* widgetComb;
    UVSizingWidget* widgetUVSizing;

    QPushButton *btnHome, *btnContour, *btnGCode, *btnRun, *btnSettings, *btnRec, *btnPult, *btnCenter, *btn8, *btn9, *btnTest, *btnMinimize, *btnShutdown, *btnHelp;
    QGridLayout* gridButtons;
    std::vector<QPushButton*> buttons;

    QTreeWidgetItem *itemWelcome, *itemSlicing, *itemComb, *itemUVSizing;

    const QString TITLE = tr("Meatec CNC Machine");
    const QString MODEL_NAME = tr("Application: ") + CNC_APP_MODEL + " " + tr("on") + " " + __DATE__ + " " + __TIME__ + CNC_APP_TYPE;
    const QString info_header = R"(<h1 style="color:black; text-align:center;">)" + TITLE + R"(</h1>)"
                            + R"(<h3 style="color:black; text-align:center;">)" + MODEL_NAME + R"(</h3>)";
//    const QString info_header = R"(<h1 style="color:black; text-align:center;">)" + TITLE + R"(</h1>)";

    const QString help_file = "home.html";

    void createButtons();

    void showConnecting(int attempt);
    void showComNotFound();
    void showNoConnection();
    void showVersion(const std::string &mcu, const std::string &fpga);

public:
    explicit FormHome(ProgramParam& par, QWidget *parent = nullptr);
    ~FormHome();

    void setFontPointSize(int pointSize);

signals:
    void contourPageClicked();
    void editPageClicked(bool plot = false);
    void runPageClicked(bool backup = false);
    void testPageClicked();
    void settingsPageClicked();
    void pultPageClicked();
    void centerPageClicked();
    void helpPageClicked(const QString& file_name);
    void programMinimize();

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

public slots:
    void connectCnc(bool reset = true);
};

#endif // FORMHOME_H
