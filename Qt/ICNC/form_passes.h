#ifndef FORM_PASSES_H
#define FORM_PASSES_H

#include <QWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QTableView>
#include "program_param.h"

class FormPasses : public QWidget {
    Q_OBJECT

    const QString help_file = "passes.html";
    ProgramParam& par;

    QRadioButton *leftSide, *rightSide;
    QGroupBox* groupSide;

    QLabel *labelTimes, *labelOverlap, *labelTab;
    QLabel** labelPasses;
    QLabel** labelModes;

    QLabel *labelTabOffset, *labelTabMode;

    QComboBox *comboTimes;
    QDoubleSpinBox *inputOverlap, *inputTab;

    QDoubleSpinBox** inputOffsets;
    QComboBox** comboModes;

    QLabel* labelCutMode;
    QComboBox* comboCutMode;

    QRadioButton *onePassTab, *multiPassTab;
    QGroupBox *groupTab;
    QDoubleSpinBox* inputTabOffset;
    QComboBox* comboTabMode;
    QCheckBox* checkTabPause;

    QLabel* labelPumpDelay;
    QSpinBox* inputPumpDelay;
    QCheckBox* checkPumpPause;

    QPushButton *btnBack, *btnOpen, *btnSave, *btnSaveAs, *btnDefault, *btn5, *btn6, *btn7, *btn8, *btn9, *btn10, *btn11, *btn12, *btnHelp;
    QGridLayout* gridButtons;

    QGridLayout *gridPasses, *gridPassMode;
    QVBoxLayout* mainLayout;

    QLabel* tableTitle;
    QTableView* tableMode;
    QPushButton *btnNewMode, *btnDeleteMode, *btnOpenModes, *btnSaveModes, *btnSaveAsModes, *btnDefaultModes;
    QVBoxLayout *vboxEdit;
    QGridLayout* gridMode;

    std::vector<QPushButton*> buttons;
    std::vector<QLabel*> labels;
    std::vector<QRadioButton*> radio;
    std::vector<QComboBox*> combo;
    std::vector<QCheckBox*> checks;
    std::vector<QDoubleSpinBox*> nums;

    size_t m_pass_num;
    int m_row;

    void createPasses();
    void initPasses();

    void resizeModeView();
    void createModeView();
    void createButtons();

    void initComboMode();
    void setCutTimes(int times);

    void on_btnOpen_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();

    void on_btnOpenModes_clicked();
    void on_btnSaveModes_clicked();
    void on_btnSaveAsModes_clicked();

    void bindModesFile();
    void unbindModesFile();
    bool bindedModesFile() const;

private slots:
    void onTableModeClicked(const QModelIndex& index);

public:
//    explicit FormPasses(QWidget *parent = nullptr);
    FormPasses(ProgramParam& par, QWidget *parent = nullptr);
    ~FormPasses();

    void setFontPointSize(int pointSize);

signals:
    void backPageClicked();
    void helpPageClicked(const QString& file_name);
};

#endif // FORM_PASSES_H
