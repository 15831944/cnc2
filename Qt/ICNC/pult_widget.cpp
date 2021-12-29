#include "pult_widget.h"
#include <QDebug>
#include <QList>
#include "program_param.h"
#include <cmath>

using namespace std;

PultWidget::PultWidget(QWidget *parent) :
    QWidget(parent)
{
    scale[0] = X_SCALE;
    scale[1] = Y_SCALE;
    scale[2] = U_SCALE;
    scale[3] = V_SCALE;
    scale_enc[0] = X_ENC_SCALE;
    scale_enc[1] = Y_ENC_SCALE;

    createView();    
    createControl();

    buttons = {btnGo, btnSet, btnApply, btnCancel, btnHold};
//    setFontPointSize(14);

    gridMain = new QGridLayout;
    gridMain->addLayout(gridControl, 0, 0, Qt::AlignTop | Qt::AlignHCenter);
    gridMain->addLayout(gridView, 0, 1, Qt::AlignTop | Qt::AlignRight);
//    gridMain->addLayout(gridButton, 1, 1, Qt::AlignRight | Qt::AlignBottom);

    this->setLayout(gridMain);

    for (size_t i = 0; i < CncParam::AXES_NUM; i++)
        setMotorView(i, 0);

    for (size_t i = 0; i < 2; i++)
        setEncoderView(i, 0);
}

int32_t PultWidget::getMoveN(size_t i) const {
    if (i < CncParam::AXES_NUM && i < sizeof(scale)/sizeof(scale[0])) {
        double res = 0;

        switch (moveMode) {
        case MoveMode::MM:
            res = moveNum[i]->value() * scale[i];
            break;
        case MoveMode::STEPS:
            res = moveNum[i]->value();
            break;
        }

        if (res > INT32_MAX)
            res = INT32_MAX;
        else if (res < INT32_MIN)
            res = INT32_MIN;

        return static_cast<int32_t>(round(res));
    }

    return 0;
}

int32_t PultWidget::getSetN(size_t i) const {
    if (i < CncParam::AXES_NUM && i < sizeof(scale)/sizeof(scale[0])) {
        double res = 0;

        switch (moveMode) {
        case MoveMode::MM:
            res = setNum[i]->value() * scale[i];
            break;
        case MoveMode::STEPS:
            res = setNum[i]->value();
            break;
        }

        if (res > INT32_MAX)
            res = INT32_MAX;
        else if (res < INT32_MIN)
            res = INT32_MIN;

        return static_cast<int32_t>(round(res));
    }

    return 0;
}

int32_t PultWidget::getSetEncN(size_t i) const {
    if (i < CncParam::AXES_NUM && i < sizeof(scale)/sizeof(scale[0]) && i < sizeof(scale_enc)/sizeof(scale_enc[0])) {
        double res = 0;

        switch (moveMode) {
        case MoveMode::MM:
            if (scale_enc[i] > 1)
                res = setNum[i]->value() * scale_enc[i];
            break;
        case MoveMode::STEPS:
            if (scale[i] > 1)
                res = setNum[i]->value() * scale_enc[i] / scale[i];
            break;
        }

        if (res > INT32_MAX)
            res = INT32_MAX;
        else if (res < INT32_MIN)
            res = INT32_MIN;

        return static_cast<int32_t>(round(res));
    }

    return 0;
}

void PultWidget::createView() {
    QLabel* labelX = new QLabel(R"(<font color=blue>X</font>)");
    QLabel* labelY = new QLabel(R"(<font color=blue>Y</font>)");
    QLabel* labelU = new QLabel(R"(<font color=blue>U</font>)");
    QLabel* labelV = new QLabel(R"(<font color=blue>V</font>)");
    QLabel* labelX2 = new QLabel(labelX->text());
    QLabel* labelY2 = new QLabel(labelY->text());
    QLabel* labelU2 = new QLabel(labelU->text());
    QLabel* labelV2 = new QLabel(labelV->text());
    QLabel* labelX3 = new QLabel(labelX->text());
    QLabel* labelY3 = new QLabel(labelY->text());
    QLabel* labelX4 = new QLabel(labelX->text());
    QLabel* labelY4 = new QLabel(labelY->text());

    QLabel* labelSpace = new QLabel("  ");

    QLabel* labelMotor = new QLabel(tr("Motor"));
    QLabel* labelEncoder = new QLabel(tr("Encoder"));
    QLabel* labelMM = new QLabel(tr("mm"));
    QLabel* labelSteps = new QLabel(tr("steps"));
    QLabel* labelMM2 = new QLabel(labelMM->text());
    QLabel* labelSteps2 = new QLabel(labelSteps->text());

    posLabels = {
        new QLabel, new QLabel, new QLabel, new QLabel,
        new QLabel, new QLabel, new QLabel, new QLabel,
        new QLabel, new QLabel,
        new QLabel, new QLabel,
        labelX, labelY, labelU, labelV,
        labelX2, labelY2, labelU2, labelV2,
        labelX3, labelY3, labelX4, labelY4,
        labelSpace,
        labelMotor, labelEncoder
    };

    posLabelsSmall = {
        labelMM, labelSteps, labelMM2, labelSteps2
    };

    for (size_t i = 0; i < 12 && i < posLabels.size(); i++)
        setMotorView(i, 0);

    // Limit Switches
    labelFWD = new QLabel("FWD");
    labelREV = new QLabel("REV");
    labelWB = new QLabel("WB");
    labelALM = new QLabel("ALM");
    labelPWR = new QLabel("PWR");

    labelFB = new QLabel("FB");
    labelTO = new QLabel("TO");

    checkLabels = {labelTO, labelFB, labelPWR, labelALM, labelWB, labelREV, labelFWD};

    checkPWR = new QCheckBox;
    checkPWR->setEnabled(false);
    checkFB = new QCheckBox;
    checkFB->setEnabled(false);
    checkTO = new QCheckBox;
    checkTO->setEnabled(false);
    checkWB = new QCheckBox;
    checkWB->setEnabled(false);
    checkALM = new QCheckBox;
    checkALM->setEnabled(false);
    checkFWD = new QCheckBox;
    checkFWD->setEnabled(false);
    checkREV = new QCheckBox;
    checkREV->setEnabled(false);

    //
    QGridLayout *gridLS = new QGridLayout;

#ifndef STONE
    gridLS->addWidget(labelTO, 0, 0, Qt::AlignCenter);
    gridLS->addWidget(checkTO, 1, 0, Qt::AlignCenter);

    gridLS->addWidget(labelFB, 0, 1, Qt::AlignCenter);
    gridLS->addWidget(checkFB, 1, 1, Qt::AlignCenter);
#endif

    gridLS->addWidget(labelPWR, 0, 2, Qt::AlignCenter);
    gridLS->addWidget(checkPWR, 1, 2, Qt::AlignCenter);

    gridLS->addWidget(labelWB, 0, 3, Qt::AlignCenter);
    gridLS->addWidget(checkWB, 1, 3, Qt::AlignCenter);

    gridLS->addWidget(labelALM, 0, 4, Qt::AlignCenter);
    gridLS->addWidget(checkALM, 1, 4, Qt::AlignCenter);

    gridLS->addWidget(labelREV, 0, 5, Qt::AlignCenter);
    gridLS->addWidget(checkREV, 1, 5, Qt::AlignCenter);

    gridLS->addWidget(labelFWD, 0, 6, Qt::AlignCenter);
    gridLS->addWidget(checkFWD, 1, 6, Qt::AlignCenter);

    // ADC
    labelGap = new QLabel(tr("Discharge Gap") + ": ");
    labelRev = new QLabel(tr("Reverse Voltage") + ": ");
    labelWorkpiece = new QLabel(tr("Workpiece") + ": ");
    labelWire = new QLabel(tr("Wire") + ": ");
    labelHV = new QLabel(tr("High Voltage") + ": ");
    labelShunt = new QLabel(tr("Shunt") + ": ");
    adcLabels = {labelGap, labelRev, labelWorkpiece, labelWire, labelHV, labelShunt};
    adcValueLabels = {new QLabel, new QLabel, new QLabel, new QLabel, new QLabel, new QLabel};

    QGridLayout* gridAdc = new QGridLayout;

    gridAdc->addWidget(labelGap, 0, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[0], 0, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelRev, 1, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[1], 1, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelWorkpiece, 2, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[2], 2, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelWire, 3, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[3], 3, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelHV, 4, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[4], 4, 1, Qt::AlignLeft);

    gridAdc->addWidget(labelShunt, 5, 0, Qt::AlignLeft);
    gridAdc->addWidget(adcValueLabels[5], 5, 1, Qt::AlignLeft);

    for (size_t i = 0; i < adcValueLabels.size(); i++)
        setAdc(i, 0);

    // Buttons
    btnCancel = new QPushButton(tr("Cancel"));
    btnCancel->setStatusTip(tr("Cancel current process"));

    btnHold = new QPushButton(tr("Hold"));
    btnHold->setCheckable(true);
    btnHold->setStatusTip(tr("Holding of stepper motors on/off"));

    // Layout
    gridView = new QGridLayout;

    gridView->addWidget(labelMotor, 0, 0, 1, 5, Qt::AlignHCenter);
    gridView->addWidget(labelMM, 1, 1, Qt::AlignHCenter);
    gridView->addWidget(labelSteps, 1, 4, Qt::AlignHCenter);

    gridView->addWidget(labelX, 2, 0);
    gridView->addWidget(posLabels[0], 2, 1, Qt::AlignRight);
    gridView->addWidget(labelSpace, 2, 2);

    gridView->addWidget(labelY, 3, 0);
    gridView->addWidget(posLabels[1], 3, 1, Qt::AlignRight);

    gridView->addWidget(labelX2, 2, 3);
    gridView->addWidget(posLabels[4], 2, 4, Qt::AlignRight);

    gridView->addWidget(labelY2, 3, 3);
    gridView->addWidget(posLabels[5], 3, 4, Qt::AlignRight);

#ifndef STONE
    gridView->addWidget(labelU, 4, 0);
    gridView->addWidget(posLabels[2], 4, 1, Qt::AlignRight);

    gridView->addWidget(labelV, 5, 0);
    gridView->addWidget(posLabels[3], 5, 1, Qt::AlignRight);

    gridView->addWidget(labelU2, 4, 3);
    gridView->addWidget(posLabels[6], 4, 4, Qt::AlignRight);

    gridView->addWidget(labelV2, 5, 3);
    gridView->addWidget(posLabels[7], 5, 4, Qt::AlignRight);

    gridView->addWidget(labelEncoder, 6, 0, 1, 5, Qt::AlignHCenter);
    gridView->addWidget(labelMM2, 7, 1, Qt::AlignHCenter);
    gridView->addWidget(labelSteps2, 7, 4, Qt::AlignHCenter);

    gridView->addWidget(labelX3, 8, 0);
    gridView->addWidget(posLabels[8], 8, 1, Qt::AlignRight);

    gridView->addWidget(labelY3, 9, 0);
    gridView->addWidget(posLabels[9], 9, 1, Qt::AlignRight);

    gridView->addWidget(labelX4, 8, 3);
    gridView->addWidget(posLabels[10], 8, 4, Qt::AlignRight);

    gridView->addWidget(labelY4, 9, 3);
    gridView->addWidget(posLabels[11], 9, 4, Qt::AlignRight);
#endif

    gridView->addLayout(gridLS, 10, 0, 1, 5, Qt::AlignRight);

#ifndef STONE
    gridView->addLayout(gridAdc, 11, 0, 1, 5, Qt::AlignRight);
#endif

    gridView->addItem(new QSpacerItem(32, 32, QSizePolicy::Preferred, QSizePolicy::Expanding), 12, 0, 1, 5);

    gridView->addWidget(btnCancel, 13, 4);
    gridView->addWidget(btnHold, 14, 4);

    gridView->setSizeConstraint(QLayout::SetFixedSize);
}

QGroupBox* PultWidget::groupLabelNum(QLabel* label, QDoubleSpinBox* num) {
    QGroupBox* group = new QGroupBox;
    label->setBuddy(num);
    group->setLayout(new QHBoxLayout);
    group->layout()->addWidget(label);
    group->layout()->addWidget(num);
    return group;
}

void PultWidget::createControl() {
    QLabel* labeldX = new QLabel("dX:");
    QLabel* labeldY = new QLabel("dY:");
    QLabel* labeldU = new QLabel("dU:");
    QLabel* labeldV = new QLabel("dV:");

    QLabel* labelX = new QLabel("X:");
    QLabel* labelY = new QLabel("Y:");
    QLabel* labelU = new QLabel("U:");
    QLabel* labelV = new QLabel("V:");
    QLabel* labelX2 = new QLabel("X:");
    QLabel* labelY2 = new QLabel("Y:");
    QLabel* labelU2 = new QLabel("U:");
    QLabel* labelV2 = new QLabel("V:");
    QLabel* labelX3 = new QLabel("X:");
    QLabel* labelY3 = new QLabel("Y:");

    QLabel* labelScale = new QLabel(tr("Scale (steps/mm)"));
    QLabel* labelMotor = new QLabel(tr("Motor"));
    QLabel* labelEncoder = new QLabel(tr("Encoder"));

    ctrlLabels = {
        labeldX, labeldY, labeldU, labeldV,
        labelX, labelY, labelU, labelV,
        labelX2, labelY2, labelU2, labelV2,
        labelX3, labelY3,
        labelScale, labelMotor, labelEncoder
    };

    /////////////////////////////////
    btnGo = new QPushButton(tr("Move"));
    btnGo->setStatusTip(tr("Relative movement on dX, dY with given speed"));

    btnSet = new QPushButton(tr("Set"));
    btnSet->setStatusTip(tr("Set position X, Y"));

    btnApply = new QPushButton(tr("Apply"));
    btnApply->setStatusTip(tr("Apply scale"));

    createMmSteps();

    numMoveX = new QDoubleSpinBox;
    numMoveY = new QDoubleSpinBox;
    numMoveU = new QDoubleSpinBox;
    numMoveV = new QDoubleSpinBox;

    numSetX = new QDoubleSpinBox;
    numSetY = new QDoubleSpinBox;
    numSetU = new QDoubleSpinBox;
    numSetV = new QDoubleSpinBox;

    moveNum = {numMoveX, numMoveY, numMoveU, numMoveV};
    setNum = {numSetX, numSetY, numSetU, numSetV};
    controlNum = {
        numMoveX, numMoveY, numMoveU, numMoveV,
        numSetX, numSetY, numSetU, numSetV
    };

    numMoveX->setRange(-1000, 1000);
    numMoveX->setSingleStep(0.001);

    //
    numScaleX = new QDoubleSpinBox;
    numScaleY = new QDoubleSpinBox;
    numScaleU = new QDoubleSpinBox;
    numScaleV = new QDoubleSpinBox;
    numScaleEncX = new QDoubleSpinBox;
    numScaleEncY = new QDoubleSpinBox;

    scaleNum = {numScaleX, numScaleY, numScaleU, numScaleV};
    encoderScaleNum = {numScaleEncX, numScaleEncY};

    createSpeed();

    gridControl = new QGridLayout;

    gridControl->addWidget(btnGo, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    gridControl->addWidget(groupLabelNum(labeldX, numMoveX), 0, 1);
    gridControl->addWidget(groupLabelNum(labeldY, numMoveY), 0, 2);

#ifndef STONE
    gridControl->addWidget(groupLabelNum(labeldU, numMoveU), 1, 1);
    gridControl->addWidget(groupLabelNum(labeldV, numMoveV), 1, 2);
#endif

    gridControl->addLayout(gridSpeed, 0, 3, 2, 1);

    gridControl->addWidget(btnSet, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    gridControl->addWidget(groupLabelNum(labelX, numSetX), 2, 1);
    gridControl->addWidget(groupLabelNum(labelY, numSetY), 2, 2);

#ifndef STONE
    gridControl->addWidget(groupLabelNum(labelU, numSetU), 3, 1);
    gridControl->addWidget(groupLabelNum(labelV, numSetV), 3, 2);
#endif

    gridControl->addWidget(groupMove, 4, 2, 1, 1, Qt::AlignHCenter);

    gridControl->addWidget(labelScale, 5, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

    gridControl->addWidget(labelMotor, 6, 0);
    gridControl->addWidget(groupLabelNum(labelX2, numScaleX), 6, 1);
    gridControl->addWidget(groupLabelNum(labelY2, numScaleY), 6, 2);

#ifndef STONE
    gridControl->addWidget(groupLabelNum(labelU2, numScaleU), 7, 1);
    gridControl->addWidget(groupLabelNum(labelV2, numScaleV), 7, 2);

    gridControl->addWidget(labelEncoder, 8, 0);
    gridControl->addWidget(groupLabelNum(labelX3, numScaleEncX), 8, 1);
    gridControl->addWidget(groupLabelNum(labelY3, numScaleEncY), 8, 2);
#endif

    gridControl->addWidget(btnApply, 9, 2);

    gridControl->addWidget(new QFrame, 0, 4, 9, 1);
    gridControl->addWidget(new QFrame, 10, 0, 1, 4);

    // Move set
    moveMode = MoveMode::MM;

    for (size_t i = 0; i < controlNum.size(); i++) {
        controlNum[i]->setRange(-999.999, 999.999);
        controlNum[i]->setSingleStep(0.001);
        controlNum[i]->setDecimals(3);
        controlNum[i]->setValue(0);
    }

    connect(moveMM, &QRadioButton::clicked, this, [&]() {
        if (moveMode != MoveMode::MM) {
            moveMode = MoveMode::MM;

            for (size_t i = 0; i < controlNum.size(); i++) {
                controlNum[i]->blockSignals(true);

                double value = controlNum[i]->value();
                controlNum[i]->setValue(0);

                controlNum[i]->setRange(-999.999, 999.999);
                controlNum[i]->setSingleStep(0.001);
                controlNum[i]->setDecimals(3);

                size_t ii = i % (sizeof(scale)/sizeof(scale[0]));
                double k = scale[ii];

                if (k > 1)
                    controlNum[i]->setValue(value / k);

                controlNum[i]->blockSignals(false);
            }
        }
    });
    connect(moveSteps, &QRadioButton::clicked, this, [&]() {
        if (moveMode != MoveMode::STEPS) {
            moveMode = MoveMode::STEPS;

            for (size_t i = 0; i < controlNum.size(); i++) {
                controlNum[i]->blockSignals(true);

                double value = controlNum[i]->value();
                controlNum[i]->setValue(0);

                controlNum[i]->setRange(-999999, 999999);
                controlNum[i]->setSingleStep(1);
                controlNum[i]->setDecimals(0);

                size_t ii = i % (sizeof(scale)/sizeof(scale[0]));
                double k = scale[ii];

                if (k > 1)
                    controlNum[i]->setValue(value * k);

                controlNum[i]->blockSignals(false);
            }
        }
    });

    // Scale
    for (size_t i = 0; i < scaleNum.size() && i < sizeof(scale) / sizeof(scale[0]); i++) {
        scaleNum[i]->setRange(1, 10000);
        scaleNum[i]->setSingleStep(1);
        scaleNum[i]->setDecimals(0);
        scaleNum[i]->setValue(scale[i]);
    }

    for (size_t i = 0; i < encoderScaleNum.size() && i < sizeof(scale_enc) / sizeof(scale_enc[0]); i++) {
        encoderScaleNum[i]->setRange(1, 10000);
        encoderScaleNum[i]->setSingleStep(1);
        encoderScaleNum[i]->setDecimals(0);
        encoderScaleNum[i]->setValue(scale_enc[i]);
    }

    connect(btnApply, &QPushButton::clicked, this, [&]() {
        scale[0] = scaleNum[0]->value();
        scale[1] = scaleNum[1]->value();
        scale[2] = scaleNum[2]->value();
        scale[3] = scaleNum[3]->value();
        scale_enc[0] = encoderScaleNum[0]->value();
        scale_enc[1] = encoderScaleNum[1]->value();
    });
}

void PultWidget::createMmSteps() {
    moveMM = new QRadioButton(tr("mm"));
    moveSteps = new QRadioButton(tr("steps"));

    groupMove = new QGroupBox;
    groupMove->setLayout(new QHBoxLayout);
    groupMove->layout()->addWidget(moveMM);
    groupMove->layout()->addWidget(moveSteps);

    groupMove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    moveMM->click();
}

void PultWidget::createSpeed() {
    labelSpeed = new QLabel(tr("Speed") + ": ");

    numSpeed = new QDoubleSpinBox;
    numSpeed->setRange(0, 100);
    numSpeed->setSingleStep(0.1);

    speedMMM = new QRadioButton(tr("mm/min"));
    speedUMS = new QRadioButton(tr("um/sec"));

    groupSpeed = new QGroupBox;
    groupSpeed->setLayout(new QHBoxLayout);
    groupSpeed->layout()->addWidget(speedMMM);
    groupSpeed->layout()->addWidget(speedUMS);

    groupSpeed->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    speedMMM->click();

    gridSpeed = new QGridLayout;
    gridSpeed->addWidget(groupLabelNum(labelSpeed, numSpeed), 0, 0);
    gridSpeed->addWidget(groupSpeed, 0, 1);
}

// mm/min
double PultWidget::speed() const {
    double value = numSpeed->value();

    if (speedUMS->isChecked())
        value *= 60.0 / 1000.0;

    return value;
}

// Limit switches
void PultWidget::setLimitSwitches(CncLimitSwitches ls) {
    checkFWD->setChecked(ls.limsw_fwd);
    checkREV->setChecked(ls.limsw_rev);
    checkALM->setChecked(ls.limsw_alm);
    checkWB->setChecked(ls.wire_break);
    checkPWR->setChecked(ls.pwr);
    checkFB->setChecked(ls.fb);
    checkTO->setChecked(ls.fb_to);
}

void PultWidget::setLimitSwitches(bool fwd, bool rev, bool alarm, bool wire_break, bool pwr, bool feedback, bool fb_timeout) {
    CncLimitSwitches ls;

    ls.limsw_fwd = fwd;
    ls.limsw_rev = rev;
    ls.limsw_alm = alarm;
    ls.wire_break = wire_break;
    ls.pwr = pwr;
    ls.fb = feedback;
    ls.fb_to = fb_timeout;

    setLimitSwitches(ls);
}

// ADC
void PultWidget::setAdc(size_t adc_num, double value, bool mV_ena) {
    if (adc_num < adcValueLabels.size()) {
        if (value < 0)
            value = 0;

        if (mV_ena)
            value *= 1e3;

        int data = round(value);

        QString s;

        if (mV_ena)
            s = QString::asprintf("%d mV", data);
        else
            s = QString::asprintf("%d V", data);

        adcValueLabels[adc_num]->setText(s);
    }
}

void PultWidget::setAdc(size_t adc_num, double_valid_t value, bool mV_ena) {
//    if (!value.valid)
//        value.value = 0;

    setAdc(adc_num, value.value, mV_ena);
}

void PultWidget::setAdc(const cnc_adc_volt_t &adc) {
    setAdc(0, adc.gap);
    setAdc(1, adc.rev_gap);
    setAdc(2, adc.workpiece);
    setAdc(3, adc.wire);
    setAdc(4, adc.hv);
    setAdc(5, adc.shunt, true);
}

//
void PultWidget::setFontPointSize(QWidget* w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void PultWidget::setFontPointSize(int pointSize) {
    QFont font;

    font = buttons[0]->font();
    font.setPointSize(pointSize);
    for (QPushButton* b: buttons)
        b->setFont(font);

    for (QLabel* label: posLabels) {
        label->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
        font = label->font();
        font.setPointSize(pointSize * 2);
        font.setBold(true);
        label->setFont(font);
    }

    font.setPointSize(static_cast<int>(round(pointSize * 0.8)));
    for (QLabel* lbl: posLabelsSmall)
        lbl->setFont(font);

    for (QLabel* o: checkLabels)
        setFontPointSize(o, pointSize);

    for (QLabel* o: adcLabels)
        setFontPointSize(o, pointSize);

    for (QLabel* o: adcValueLabels)
        setFontPointSize(o, pointSize);

    font = btnCancel->font();
    font.setPointSize(pointSize);
    btnCancel->setFont(font);
    btnHold->setFont(font);

    font = ctrlLabels[0]->font();
    font.setPointSize(static_cast<int>(round(pointSize * 1.2)));
    for (QLabel* label: ctrlLabels)
        label->setFont(font);

    font = btnGo->font();
    font.setPointSize(pointSize);

    btnGo->setFont(font);
    btnSet->setFont(font);
    btnApply->setFont(font);

    font = numMoveX->font();
    font.setPointSize(pointSize);

    numMoveX->setFont(font);
    numMoveY->setFont(font);
    numMoveU->setFont(font);
    numMoveV->setFont(font);
    numSetX->setFont(font);
    numSetY->setFont(font);
    numSetU->setFont(font);
    numSetV->setFont(font);

    numScaleX->setFont(font);
    numScaleY->setFont(font);
    numScaleU->setFont(font);
    numScaleV->setFont(font);
    numScaleEncX->setFont(font);
    numScaleEncY->setFont(font);

    font = moveMM->font();
    font.setPointSize(static_cast<int>(round(pointSize * 0.8)));
    moveMM->setFont(font);
    moveSteps->setFont(font);

    font = labelSpeed->font();
    font.setPointSize(pointSize);
    labelSpeed->setFont(font);

    font = numSpeed->font();
    font.setPointSize(pointSize);
    numSpeed->setFont(font);

    font = speedMMM->font();
    font.setPointSize(static_cast<int>(round(pointSize * 0.8)));
    speedMMM->setFont(font);
    speedUMS->setFont(font);
}

PultWidget::~PultWidget() {}

void PultWidget::setMotorView(size_t axis_num, int value) {
    if (axis_num < CncParam::AXES_NUM) {
        bool sign = value < 0;
        unsigned abs_value = std::abs(value);

        double k = axis_num < sizeof(scale)/sizeof(scale[0]) ? scale[axis_num] : 1.0;

        uint32_t value_um = static_cast<uint32_t>( std::round(double(abs_value) / k * 1e3) );

        QString s = QString::asprintf("%c%04d.%03d", sign ? '-' : ' ', value_um / 1000, value_um % 1000);
        QString s2 = QString::asprintf("%c%07d", sign ? '-' : ' ', abs_value);

        posLabels[axis_num]->setText(R"(<font color=red>)" + s + R"(</font>)");
        posLabels[axis_num + CncParam::AXES_NUM]->setText(R"(<font color=red>)" + s2 + R"(</font>)");
    }
}

void PultWidget::setEncoderView(size_t axis_num, int value) {
    if (axis_num < CncParam::ENCODERS_NUM) {
        bool sign = value < 0;
        unsigned abs_value = std::abs(value);

        double k = axis_num < sizeof(scale_enc)/sizeof(scale_enc[0]) ? scale_enc[axis_num] : 1.0;

        uint32_t value_um = static_cast<uint32_t>( std::round(double(abs_value) / k * 1e3) );

        QString s = QString::asprintf("%c%04d.%03d", sign ? '-' : ' ', value_um / 1000, value_um % 1000);
        QString s2 = QString::asprintf("%c%07d", sign ? '-' : ' ', abs_value);

        posLabels[axis_num + 8]->setText(R"(<font color=red>)" + s + R"(</font>)");
        posLabels[axis_num + 8 + 2]->setText(R"(<font color=red>)" + s2 + R"(</font>)");
    }
}
