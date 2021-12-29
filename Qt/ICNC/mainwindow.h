#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "main.h"
#include <QMainWindow>
#include "form_home.h"
#include "form_edit.h"
#include "form_run.h"
#include "form_test.h"
#include <QStackedWidget>
#include "program_param.h"
#include <QLCDNumber>
#include "form_contour.h"
#include "form_center.h"

#if defined(STONE)
    #include "form_passes_stone.h"
#else
    #include "form_passes.h"
#endif

#include "form_settings.h"
#include "form_pult.h"
#include "form_help.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
//    Q_PROPERTY(ProgramParam param READ getParam WRITE setParam)

private:
    static int cnt;

    FormHome* m_formHome;
    FormEdit* m_formEdit;
    FormRun* m_formRun;
    FormTest* m_formTest;
    FormContour* m_formContour;
    FormSettings* m_formSettings;
#if defined(STONE)
    FormPassesStone* m_formPasses;
#else
    FormPasses* m_formPasses;
#endif
    FormPult* m_formPult;
    FormCenter* m_formCenter;
    FormHelp* m_formHelp;
    QWidget* m_helpWidget;

    QStackedWidget* m_stackedWidget;
    ProgramParam par;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

//    void setParam(ProgramParam param) { par = param; }
//    ProgramParam getParam() const { return par; }

private slots:
    void onHomePageClicked();
    void onContourPageClicked();
    void onEditPageClicked(bool plot);
    void onRunPageClicked(bool backup);
    void onSettingsPageClicked();
    void onPultPageClicked();
    void onCenterPageClicked();
    void onTestPageClicked();
    void onFileNameChanged(const QString& file_name);

    void onPassesPageClicked();

    void onHelpPageClicked(QWidget* source, const QString& helpHtmlFileName);
    void onBackHelpClicked();

    void showTime();

//public slots:
//    void onLoad();

private:
    Ui::MainWindow *ui;
    QLabel *labelMeatec, *labelTitle, *labelTime;
//    QLCDNumber* lcdTime;

    void updateFontPointSize();

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void showEvent(QShowEvent *event);
};

#endif // MAINWINDOW_H
