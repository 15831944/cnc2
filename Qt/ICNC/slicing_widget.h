#ifndef SLICINGWIDGET_H
#define SLICINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <vector>

#include "my_types.h"
#include "program_param.h"

class SlicingWidget : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QLabel *labelTitle, *labelWireD, *labelWidth, *labelSpacing, *labelSlicingStep, *labelStepNum, *labelSectionsNum, *labelSpeedAvg;
    QDoubleSpinBox *fnumWireD, *fnumWidth, *fnumSpacing, *fnumSlicingStep, *fnumSpeedAvg;
    QSpinBox *numSlicesNum, *numSectionsNum;
    QGroupBox *groupProfile;
    QRadioButton *radioRect, *radioCircle;
    QPushButton *btnGen;

//    std::vector<QLabel*> labels;
//    std::vector<QDoubleSpinBox*> fNumbers;
//    std::vector<QSpinBox*> numbers;
//    std::vector<QRadioButton*> radios;

     std::vector<QWidget*> widgets;

    static void setFontPointSize(QWidget* w, int pointSize);

private slots:
    void onGenerate();

signals:
    void clicked();

public:
    explicit SlicingWidget(ProgramParam& par, QWidget *parent = nullptr);

    slicing_t slicing() const;

    void setFontPointSize(int pointSize);

signals:

};

#endif // SLICINGWIDGET_H
