#ifndef FORM_DXF_H
#define FORM_DXF_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
//#include <QSplitter>
#include <QSpacerItem>
#include <QTableView>
#include <QGroupBox>
#include <QMenu>
#include <QResizeEvent>

#include "program_param.h"
#include "contour_table_model.h"
#include "plot_view.h"
#include "qwt_plot_view.h"

class FormContour : public QWidget {
    Q_OBJECT

    const QString help_file = "contour.html";
    ProgramParam& par;

    QPushButton *btnHome, *btnPasses, *btnOpen, *btnSave, *btnSaveAs, *btnBot, *btnTop, *btnGenerate, *btn8, *btn9, *btn10, *btn11, *btn12, *btnHelp;
    QGridLayout *gridButtons;
    QVBoxLayout *mainLayout;
//    QSplitter* splitter;

    QTextEdit* txtMsg;

    QwtPlotView plotView;
    double m_plot_view_pct;

    QGridLayout *gridPlot, *gridView;
    QLineEdit *lineContours, *lineContour;
    QTableView *viewContours, *viewContour;
    QGroupBox *groupContours, *groupContour;
    QPushButton *btnNewContour, *btnAddCutline, *btnDelete, *btnClear;
    QWidget *widgetProperties;

    QVBoxLayout *vLayoutLeft, *vLayoutRight;

    QPushButton *btnBegin, *btnChangeDir;

    QPushButton *btnEdit;
    QMenu* menuEdit;
    QAction *actCutline, *actExitPoint, *actOffset, *actRotate, *actResize, *actFlipLeftRight, *actFlipUpDown;

    size_t m_contour_num, m_row, m_column;
    bool file_open;

    fpoint_valid_t center{false,0,0};

    std::vector<QPushButton*> buttons;

    void createButtons();
    void createGridView();
    void createViewControl();

    bool openDxfFileDialog(QFile& file, QString& fileDir, QString& fileName);

    void initView();
    void setEmptyModel();
    void initContourView();

    QSize plotViewSize() const;
    QSize plotViewSize(const QSize& newSize) const;

signals:
    void homePageClicked();
    void passesPageClicked();
    void editPageClicked(bool auto_plot);
    void helpPageClicked(const QString& file_name);
    void fileNameChanged(const QString& file_name);

private slots:
    void on_btnBot_clicked();
    void on_btnTop_clicked();

    void on_btnNewContour_clicked();
    void on_btnNewCutline_clicked();
    void on_btnDelete_clicked();
    void on_btnClear_clicked();

    void on_btnReverse_clicked();
    void onBeginClicked();

    void onViewContoursClicked(const QModelIndex&);
    void onViewContourClicked(const QModelIndex&);

    void on_actCutline_triggered();
    void on_actExitPoint_triggered();
    void on_actRotate_triggered();
    void on_actResize_triggered();
    void on_actFlipLeftRight_triggered();
    void on_actFlipUpDown_triggered();

    void onGenerate();

    static void setFontPointSize(QWidget *w, int pointSize);

protected:
    virtual void resizeEvent(QResizeEvent *event);

public:
//    explicit FormDxf(QWidget *parent = nullptr);
    FormContour(ProgramParam& par, QWidget *parent = nullptr);
    ~FormContour();

    void setFontPointSize(int pointSize);
    QString getFileName() const;
    void resetFileOpen();

    void init();

    void plot();
    void plot(const Dxf& contour);
};

#endif // FORM_DXF_H
