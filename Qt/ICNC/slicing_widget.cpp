#include "slicing_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>

#include "wire_cut.h"

SlicingWidget::SlicingWidget(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    labelTitle = new QLabel(R"(<h1>)" + tr("Slicing") + R"(<br></h1>)");
    labelWireD = new QLabel(tr("Wire diameter"));
    labelWidth = new QLabel(tr("Workpiece width"));
    labelSpacing = new QLabel(tr("Spacing"));
    labelSlicingStep = new QLabel(tr("Slicing step"));
    labelStepNum = new QLabel(tr("Slices number"));
    labelSectionsNum = new QLabel(tr("Sections number in the speed profile"));
    labelSpeedAvg = new QLabel(tr("Speed"));

//    labels = {labelWireD, labelWidth, labelSpacing, labelSlicingStep, labelStepNum, labelSectionsNum, labelSpeedAvg};

    //
    fnumWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(fnumWireD);
    fnumWireD->setSuffix(" " + tr("mm"));
    fnumWireD->setDecimals(3);
    fnumWireD->setSingleStep(0.001);
    fnumWireD->setRange(0.05, 3);
    fnumWireD->setValue(cut_t::DEFAULT_WIRE_DIAMETER);

    fnumWidth = new QDoubleSpinBox;
    labelWidth->setBuddy(fnumWidth);
    fnumWidth->setSuffix(" " + tr("mm"));
    fnumWidth->setDecimals(3);
    fnumWidth->setSingleStep(0.001);
    fnumWidth->setRange(1, 1000);
    fnumWidth->setValue(100);

    fnumSpacing = new QDoubleSpinBox;
    labelSpacing->setBuddy(fnumSpacing);
    fnumSpacing->setSuffix(" " + tr("mm"));
    fnumSpacing->setDecimals(1);
    fnumSpacing->setSingleStep(1);
    fnumSpacing->setRange(1, 20);
    fnumSpacing->setValue(10);

    fnumSlicingStep = new QDoubleSpinBox;
    labelSlicingStep->setBuddy(fnumSlicingStep);
    fnumSlicingStep->setSuffix(" " + tr("mm"));
    fnumSlicingStep->setDecimals(3);
    fnumSlicingStep->setSingleStep(0.001);
    fnumSlicingStep->setRange(1, 1000);
    fnumSlicingStep->setValue(10);

    fnumSpeedAvg = new QDoubleSpinBox;
    labelSpeedAvg->setBuddy(fnumSpeedAvg);
    fnumSpeedAvg->setSuffix(" " + tr("mm/min"));
    fnumSpeedAvg->setDecimals(1);
    fnumSpeedAvg->setSingleStep(1);
    fnumSpeedAvg->setRange(0.1, 18);
    fnumSpeedAvg->setValue(1);

    //
    numSlicesNum = new QSpinBox;
    labelStepNum->setBuddy(numSlicesNum);
    numSlicesNum->setRange(1, 100);
    numSlicesNum->setValue(3);

    numSectionsNum = new QSpinBox;
    labelSectionsNum->setBuddy(numSectionsNum);
    numSectionsNum->setRange(1, 100);
    numSectionsNum->setValue(20);
    numSectionsNum->setEnabled(false);
    labelSectionsNum->setEnabled(false);

    //
    groupProfile = new QGroupBox(tr("Workpiece profile"));
    radioRect = new QRadioButton(tr("Rectangle"));
    radioCircle = new QRadioButton(tr("Circle"));
    {
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(radioRect);
        vbox->addWidget(radioCircle);
        groupProfile->setLayout(vbox);
        radioRect->setChecked(true);
    }

    //
    btnGen = new QPushButton(tr("Generate"));
    btnGen->setStatusTip(tr("Generate contour"));

    //
    QGridLayout* grid = new QGridLayout;
    grid->addWidget(labelTitle, 0, 0, 1, 2, Qt::AlignHCenter | Qt::AlignBottom);
    grid->addWidget(labelWireD, 1, 0);
    grid->addWidget(fnumWireD, 1, 1);
    grid->addWidget(labelWidth, 2, 0);
    grid->addWidget(fnumWidth, 2, 1);
    grid->addWidget(labelSpacing, 3, 0);
    grid->addWidget(fnumSpacing, 3, 1);
    grid->addWidget(labelSlicingStep, 4, 0);
    grid->addWidget(fnumSlicingStep, 4, 1);
    grid->addWidget(labelStepNum, 5, 0);
    grid->addWidget(numSlicesNum, 5, 1);
    grid->addWidget(groupProfile, 6, 0, 1, 2, Qt::AlignLeft | Qt::AlignTop);
    grid->addWidget(labelSpeedAvg, 7, 0);
    grid->addWidget(fnumSpeedAvg, 7, 1);
    grid->addWidget(labelSectionsNum, 8, 0);
    grid->addWidget(numSectionsNum, 8, 1);
    grid->addWidget(btnGen, 9, 1, Qt::AlignRight | Qt::AlignBottom);

    //
//    QWidget* widgetInside = new QWidget(this);
//    widgetInside->setLayout(grid);

    QGridLayout* gridCentral = new QGridLayout;
    gridCentral->addLayout(grid, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridCentral->addWidget(new QFrame, 0, 1);
    gridCentral->addWidget(new QFrame, 1, 0, 1, 2);

//    QWidget* widgetCentral = new QWidget(this);
//    widgetCentral->setLayout(gridCentral);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridCentral);
    this->setLayout(mainLayout);

    connect(radioRect, &QRadioButton::clicked, this, [&]() {
        labelSectionsNum->setEnabled(false);
        numSectionsNum->setEnabled(false);
        labelWidth->setText(tr("Workpiece width"));
        labelSpeedAvg->setText(tr("Speed"));
    });
    connect(radioCircle, &QRadioButton::clicked, this, [&]() {
        labelSectionsNum->setEnabled(true);
        numSectionsNum->setEnabled(true);
        labelWidth->setText(tr("Workpiece diameter"));
        labelSpeedAvg->setText(tr("Average speed"));
    });

    connect(btnGen, &QPushButton::clicked, this, &SlicingWidget::onGenerate);

    widgets = {
        labelTitle, labelWireD, labelWidth, labelSpacing, labelSlicingStep, labelStepNum, labelSectionsNum, labelSpeedAvg,
        fnumWireD, fnumWidth, fnumSpacing, fnumSlicingStep, fnumSpeedAvg,
        numSlicesNum, numSectionsNum,
        groupProfile,
        radioRect, radioCircle,
        btnGen
    };
}

//struct slicing_t{
//    bool rotate_ena {false};
//    unsigned step_num {0}, profile_step_num {0};
//    double wireD {0}, width {0}, spacing {0}, step {0}, speed_avg {0}, rotate {0};
//};

slicing_t SlicingWidget::slicing() const {
    slicing_t res;
    res.slices_num          = numSlicesNum->value() > 0 ? numSlicesNum->value() : 0;
    res.sections_num        = numSectionsNum->value() > 0 ? numSectionsNum->value() : 0;
    res.wireD               = fnumWireD->value() > 0 ? fnumWireD->value() : 0;
    res.width               = fnumWidth->value() > 0 ? fnumWidth->value() : 0;
    res.spacing             = fnumSpacing->value() > 0 ? fnumSpacing->value() : 0;
    res.slicing_step        = fnumSlicingStep->value() > 0 ? fnumSlicingStep->value() : 0;
    res.speed_avg           = fnumSpeedAvg->value() > 0 ? fnumSpeedAvg->value() : 0;
    res.profile             = radioCircle->isChecked() ? SLICING_PROFILE::CIRCLE : SLICING_PROFILE::RECT;
    return res;
}

void SlicingWidget::onGenerate() {
    Dxf bot;
    bot.generate(slicing());

    par.contours.clear();
    par.contours.new_back();
    par.contours.back()->setBot(bot);
    emit clicked();
}

void SlicingWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void SlicingWidget::setFontPointSize(int pointSize) {
    for (QWidget* w: widgets)
        setFontPointSize(w, pointSize);
}
