#ifndef FORM_SETTINGS_H
#define FORM_SETTINGS_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include "program_param.h"

class FormSettings : public QWidget
{
    Q_OBJECT

    const QString help_file = "settings.html";
    ProgramParam& par;

    QVBoxLayout *mainLayout;

    QPushButton *btnHome, *btnRead, *btnWrite, *btnDefault, *btn4, *btn5, *btn6, *btn7, *btn8, *btn9, *btn10, *btn11, *btn12, *btnHelp;
    QGridLayout* gridButtons;

    QWidget *widgetSettings;
    QGridLayout *gridSettings;

    QLabel *labelTitle, *labelCNC, *labelLanguage, *labelInputLevel, *labelStep, *labelX, *labelY, *labelU, *labelV, *labelEncX,\
        *labelEncY, *labelPrecision, *labelMotor, *labelEncoder, *labelAcc, *labelDec;

    QComboBox* comboLanguage;
    QCheckBox *checkReverseX, *checkReverseY, *checkSwapXY, *checkShowXY,\
        *checkStepDirEnable, *checkEncXY,\
        *checkReverseMotorX, *checkReverseMotorY, *checkReverseMotorU, *checkReverseMotorV, *checkSwapMotorXY, *checkSwapMotorUV,\
        *checkReverseEncX, *checkReverseEncY;

    QSpinBox *numInputLevel;
    QComboBox* comboInputLevel;

    QGroupBox *groupFeedback;
    QLabel *labelLowThld, *labelHighThld, *labelRbTimeout, *labelRbAttempts, *labelRbLength, *labelRbSpeed;
    QSpinBox *numLowThld, *numHighThld, *numRbTimeout, *numRbAttempts;
    QDoubleSpinBox *fnumRbLength, *fnumRbSpeed;

    QDoubleSpinBox *numStep, *numScaleX, *numScaleY, *numScaleU, *numScaleV, *numScaleEncX, *numScaleEncY, *fnumAcc, *fnumDec;

    std::vector<QDoubleSpinBox*> scaleNum;
    std::vector<QDoubleSpinBox*> encScaleNum;

    std::vector<QPushButton*> buttons;

    std::vector<QSpinBox*> spinBoxes;

    void createButtons();
    void createSettingsWidget();

    static QGroupBox* groupLabelNum(QLabel* label, QDoubleSpinBox* num);
    static QGroupBox* groupLabelNum(QLabel* label, QSpinBox* num, QComboBox* combo);
    static QGroupBox* groupLabelCombo(QLabel* label, QComboBox* combo);

    void onStateChangedMotor(int state);
    void onNumScaleChanged(double value);

//    void initSettings();

private slots:

public:
//    explicit FormSettings(QWidget *parent = nullptr);
    FormSettings(ProgramParam& par, QWidget *parent = nullptr);
    ~FormSettings();

    void setFontPointSize(int pointSize);

signals:
    void homePageClicked();
    void helpPageClicked(const QString& file_name);

    void showInfo(const QString& text);
    void showWarning(const QString& text);
    void showError(const QString& text);

    void fontChanged(int fontSize);
};

#endif // FORM_SETTINGS_H
