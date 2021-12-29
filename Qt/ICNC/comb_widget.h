#ifndef COMBWIDGET_H
#define COMBWIDGET_H

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

class CombWidget : public QWidget {
    Q_OBJECT

    ProgramParam& par;

    QLabel *labelTitle, *labelWireD, *labelDepth, *labelStep, *labelTeethNum, *labelSpeed, *labelDrumVel;
    QDoubleSpinBox *fnumWireD, *fnumDepth, *fnumStep, *fnumSpeed;
    QSpinBox *numTeethNum, *numDrumVel;
    QGroupBox *groupAxis;
    QRadioButton *radioX, *radioY;
    QPushButton *btnGen;

    std::vector<QWidget*> widgets;

    static void setFontPointSize(QWidget* w, int pointSize);

private slots:
    void onGenerate();

signals:
    void clicked();

public:
    explicit CombWidget(ProgramParam& par, QWidget *parent = nullptr);

    comb_t comb() const;

    void setFontPointSize(int pointSize);

signals:

};

#endif // COMBWIDGET_H
