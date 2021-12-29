#ifndef PULT_PLOT_WIDGET_H
#define PULT_PLOT_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>

//#include "gcode.h"
//#include "plot_view.h"
//#include "code_editor.h"
//#include "qwt_plot_view.h"
#include "cnc_context.h"

class PultWidget : public QWidget {
    Q_OBJECT

public:
    enum class MoveMode { MM, STEPS };
    MoveMode moveMode = MoveMode::MM;

    QPushButton *btnGo, *btnSet, *btnApply;

    QDoubleSpinBox *numMoveX, *numMoveY, *numMoveU, *numMoveV;
    QDoubleSpinBox *numSetX, *numSetY, *numSetU, *numSetV;
    std::vector<QDoubleSpinBox*> moveNum, setNum, controlNum;

    QRadioButton *moveMM, *moveSteps;
    QGroupBox *groupMove;

    QLabel *labelSpeed;
    QDoubleSpinBox *numSpeed;
    QRadioButton *speedMMM, *speedUMS;
    QGroupBox *groupSpeed;
    QGridLayout *gridSpeed;

    QPushButton *btnCancel, *btnHold;
    QGridLayout *gridButton;

    QDoubleSpinBox *numScaleX, *numScaleY, *numScaleU, *numScaleV;
    QDoubleSpinBox *numScaleEncX, *numScaleEncY;
    std::vector<QDoubleSpinBox*> scaleNum;
    std::vector<QDoubleSpinBox*> encoderScaleNum;
    QGridLayout *gridScale;

    std::vector<QLabel*> controlLabels;

    std::vector<QPushButton*> buttons;
    std::vector<QLabel*> labels;
    std::vector<QDoubleSpinBox*> nums;
    std::vector<QRadioButton*> radio;

    QLabel *labelTO, *labelFB, *labelPWR, *labelALM, *labelWB, *labelREV, *labelFWD;
    std::vector<QLabel*> checkLabels;
    QCheckBox *checkTO, *checkFB, *checkPWR, *checkALM, *checkWB, *checkREV, *checkFWD;

    QLabel *labelGap, *labelRev, *labelWorkpiece, *labelWire, *labelHV, *labelShunt;
    std::vector<QLabel*> adcLabels;
    std::vector<QLabel*> adcValueLabels;

    double scale[4];

private:
    std::vector<QLabel*> posLabels, posLabelsSmall, ctrlLabels;
    QGridLayout *gridView, *gridMain, *gridControl, *gridControl2;

    double scale_enc[2];

    void createView();
    void createControl();
    void createMove();
    void createMmSteps();
    void createScale();
    void createSpeed();
    void createButtons();

    static QGroupBox* groupLabelNum(QLabel* label, QDoubleSpinBox* num);

public:
    explicit PultWidget(QWidget *parent = nullptr);
    ~PultWidget();

    void setMotorView(size_t axis, int value);
    void setEncoderView(size_t axis, int value);

    void setLimitSwitches(CncLimitSwitches ls);
    void setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout);

    void setAdc(size_t adc_num, double value, bool mV_ena = false);
    void setAdc(size_t adc_num, double_valid_t value, bool mV_ena = false);
    void setAdc(const cnc_adc_volt_t& adc);

    int32_t getMoveN(size_t i) const;
    int32_t getSetN(size_t i) const;
    int32_t getSetEncN(size_t i) const;

    double speed() const;

    void setFontPointSize(QWidget* w, int pointSize);
    void setFontPointSize(int pointSize);
};

#endif // PULT_PLOT_WIDGET_H
