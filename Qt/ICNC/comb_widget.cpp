#include "comb_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>

#include "contour_pass.h"
#include "wire_cut.h"

CombWidget::CombWidget(ProgramParam& par, QWidget *parent) : QWidget(parent), par(par) {
    labelTitle = new QLabel(R"(<h1>)" + tr("Comb") + R"(<br></h1>)");
    labelWireD = new QLabel(tr("Wire diameter"));
    labelDepth = new QLabel(tr("Slots depth") + ", " + tr("signed value"));
    labelStep = new QLabel(tr("Step") + ", " + tr("signed value"));
    labelTeethNum = new QLabel(tr("Slots number"));
    labelSpeed = new QLabel(tr("Cutting speed"));
    labelDrumVel = new QLabel(tr("Drum velocity"));

    fnumWireD = new QDoubleSpinBox;
    labelWireD->setBuddy(fnumWireD);
    fnumWireD->setSuffix(" " + tr("mm"));
    fnumWireD->setDecimals(3);
    fnumWireD->setSingleStep(0.001);
    fnumWireD->setRange(0, 3);
    fnumWireD->setValue(cut_t::DEFAULT_WIRE_DIAMETER);

    fnumDepth = new QDoubleSpinBox;
    labelDepth->setBuddy(fnumDepth);
    fnumDepth->setSuffix(" " + tr("mm"));
    fnumDepth->setDecimals(3);
    fnumDepth->setSingleStep(0.001);
    fnumDepth->setRange(-1000, 1000);
    fnumDepth->setValue(-50);

    fnumStep = new QDoubleSpinBox;
    labelStep->setBuddy(fnumStep);
    fnumStep->setSuffix(" " + tr("mm"));
    fnumStep->setDecimals(3);
    fnumStep->setSingleStep(0.001);
    fnumStep->setRange(-1000, 1000);
    fnumStep->setValue(10);

    numTeethNum = new QSpinBox;
    labelTeethNum->setBuddy(numTeethNum);
    numTeethNum->setRange(1, 1000);
    numTeethNum->setValue(10);

    fnumSpeed = new QDoubleSpinBox;
    labelSpeed->setBuddy(fnumSpeed);
    fnumSpeed->setSuffix(" " + tr("mm/min"));
    fnumSpeed->setDecimals(1);
    fnumSpeed->setSingleStep(0.1);
    fnumSpeed->setRange(0.1, 18);
    fnumSpeed->setValue(1);

    numDrumVel = new QSpinBox;
    labelDrumVel->setBuddy(numDrumVel);
    numDrumVel->setRange(1, 7);
    numDrumVel->setValue(7);

    groupAxis = new QGroupBox(tr("Axis"));
    radioX = new QRadioButton("X");
    radioY = new QRadioButton("Y");
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(radioX);
        hbox->addWidget(radioY);
        groupAxis->setLayout(hbox);
        radioX->setChecked(true);
    }

    //
    btnGen = new QPushButton(tr("Generate"));
    btnGen->setStatusTip(tr("Generate contour"));

    //
    QGridLayout* grid = new QGridLayout;    
    grid->addWidget(labelTitle, 0, 0, 1, 2, Qt::AlignHCenter | Qt::AlignBottom);

    grid->addWidget(labelWireD, 1, 0);
    grid->addWidget(fnumWireD, 1, 1);

    grid->addWidget(labelDepth, 2, 0);
    grid->addWidget(fnumDepth, 2, 1);

    grid->addWidget(labelStep, 3, 0);
    grid->addWidget(fnumStep, 3, 1);

    grid->addWidget(labelTeethNum, 4, 0);
    grid->addWidget(numTeethNum, 4, 1);

    grid->addWidget(labelSpeed, 5, 0);
    grid->addWidget(fnumSpeed, 5, 1);

    grid->addWidget(labelDrumVel, 6, 0);
    grid->addWidget(numDrumVel, 6, 1);

    grid->addWidget(groupAxis, 7, 0, 1, 2, Qt::AlignLeft | Qt::AlignTop);

    grid->addWidget(btnGen, 8, 1, Qt::AlignRight | Qt::AlignBottom);

    QGridLayout* gridCentral = new QGridLayout;
    gridCentral->addLayout(grid, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    gridCentral->addWidget(new QFrame, 0, 1);
    gridCentral->addWidget(new QFrame, 1, 0, 1, 2);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridCentral);
    this->setLayout(mainLayout);

    connect(btnGen, &QPushButton::clicked, this, &CombWidget::onGenerate);

    widgets = {
        labelTitle, labelWireD, labelDepth, labelStep, labelTeethNum, labelSpeed, labelDrumVel,
        fnumWireD, fnumStep, fnumDepth, fnumSpeed,
        numTeethNum, numDrumVel,
        groupAxis,
        radioX, radioY,
        btnGen
    };
}

comb_t CombWidget::comb() const {
    comb_t res;
    res.teeth_num   = numTeethNum->value() > 0 ? numTeethNum->value() : 0;
    res.drum_vel    = numDrumVel->value() > 0 ? numDrumVel->value() : 0;
    res.wireD       = fnumWireD->value() > 0 ? fnumWireD->value() : 0;
    res.depth       = fnumDepth->value();
    res.step        = fnumStep->value();
    res.speed       = fnumSpeed->value() > 0 ? fnumSpeed->value() : 0;
    res.axis        = radioY->isChecked() ? AXIS::AXIS_Y : AXIS::AXIS_X;

    res.speed_idle  = Cnc::SPEED_MAX;
    return res;
}

void CombWidget::onGenerate() {
    Dxf bot;
    bot.generate(comb());

    par.contours.clear();
    par.contours.new_back();
    par.contours.back()->setBot(bot);

    ContourPair* pair = par.contours.front();
    std::deque<ContourPair> ar;
    ar.push_back(*pair);

    if (par.gcode.generate( par.cut, ar)) {
        par.gcodeText = par.gcode.toText().c_str();
        par.gcodeSettings = par.gcode.getSettings();

        if (!par.gcode.empty())
            emit clicked();
    }
}

void CombWidget::setFontPointSize(QWidget *w, int pointSize) {
    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

void CombWidget::setFontPointSize(int pointSize) {
    for (QWidget* w: widgets)
        setFontPointSize(w, pointSize);
}
