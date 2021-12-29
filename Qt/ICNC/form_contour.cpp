#include "form_contour.h"

#include <QFileDialog>
//#include <QSizePolicy>
#include <QLineEdit>
#include <QHeaderView>

#include "new_cutline_dialog.h"
#include "rotate_dialog.h"
#include "resize_dialog.h"
#include "contour_pass.h"
#include "frect_t.h"

using namespace std;
//using namespace QtCharts;

FormContour::FormContour(ProgramParam& par, QWidget *parent) :
    QWidget(parent),
    par(par), plotView(QwtPlotView()),
    m_contour_num(0), m_row(0), m_column(0),
    file_open(false)
{
    createButtons();

    txtMsg = new QTextEdit();
    txtMsg->setReadOnly(true);

//    QSizePolicy spMsg(QSizePolicy::Maximum, QSizePolicy::Maximum);
//    spMsg.setHeightForWidth(true);
//    txtMsg->setSizePolicy(spMsg);

    createGridView();

    buttons = {btnHome, btnPasses, btnOpen, btnSave, btnSaveAs, btnBot, btnTop, btnGenerate,
               btn8, btn9, btn10, btn11, btn12, btnHelp, btnNewContour, btnAddCutline,
               btnDelete, btnClear, btnBegin, btnChangeDir, btnEdit
              };

//    setFontPointSize(14);

    gridPlot = new QGridLayout();
    gridPlot->addLayout(gridView, 0, 0);
//    gridPlot->addWidget(plotView.give(), 0, 1, Qt::AlignLeft | Qt::AlignBottom);
    gridPlot->addWidget(plotView.widget(), 0, 1);
    gridPlot->addWidget(txtMsg, 1, 0, 1, 2);

    m_plot_view_pct = (14.0 - 3) / 14;

    gridPlot->setColumnStretch(0, 3);
    gridPlot->setColumnStretch(1, 14 - 3);

    gridPlot->setRowStretch(0, 9);
    gridPlot->setRowStretch(1, 1);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridPlot);
    mainLayout->addLayout(gridButtons);

    this->setLayout(mainLayout);
}

void FormContour::createButtons() {
#if !defined(STONE)
    btnHome = new QPushButton(tr("Home"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setEnabled(false);

    btnSave = new QPushButton(tr("Save"));
    btnSave->setEnabled(false);

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setEnabled(false);

    btnBot = new QPushButton(tr("Load XY"));

    btnTop = new QPushButton(tr("Load UV"));
    btnTop->setEnabled(false);

    btnPasses = new QPushButton(tr("Passes"));

    btnGenerate = new QPushButton(tr("Generate"));

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btn12 = new QPushButton;
    btn12->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnHome, 0, 0);    
    gridButtons->addWidget(btnOpen, 0, 1);
    gridButtons->addWidget(btnSave, 0, 2);
    gridButtons->addWidget(btnSaveAs, 0, 3);
    gridButtons->addWidget(btnBot, 0, 4);
    gridButtons->addWidget(btnTop, 0, 5);
    gridButtons->addWidget(btnPasses, 0, 6);
    gridButtons->addWidget(btnGenerate, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);

    gridButtons->addWidget(btnHelp, 0, 13);
#else
    btnHome = new QPushButton(tr("Home"));
    btnHome->setStatusTip(tr("Go to the home panel"));

    btnOpen = new QPushButton(tr("Open"));
    btnOpen->setStatusTip(tr("Open project"));
    btnOpen->setEnabled(false);

    btnSave = new QPushButton(tr("Save"));
    btnSave->setStatusTip(tr("Save current project"));
    btnSave->setEnabled(false);

    btnSaveAs = new QPushButton(tr("Save as"));
    btnSaveAs->setStatusTip(tr("Save current project with a new name"));
    btnSaveAs->setEnabled(false);

    btnBot = new QPushButton(tr("Load DXF"));
    btnBot->setStatusTip(tr("Load DXF file"));

    btnTop = new QPushButton;
    btnTop->setEnabled(false);

    btnPasses = new QPushButton(tr("Adjust"));
    btnPasses->setStatusTip(tr("Adjust contour"));

    btnGenerate = new QPushButton(tr("Generate"));
    btnGenerate->setStatusTip(tr("Generate G-code"));

    btn8 = new QPushButton;
    btn8->setEnabled(false);

    btn9 = new QPushButton;
    btn9->setEnabled(false);

    btn10 = new QPushButton;
    btn10->setEnabled(false);

    btn11 = new QPushButton;
    btn11->setEnabled(false);

    btn12 = new QPushButton;
    btn12->setEnabled(false);

    btnHelp = new QPushButton(tr("Help"));

    gridButtons = new QGridLayout();

    gridButtons->addWidget(btnHome, 0, 0);
    gridButtons->addWidget(btnOpen, 0, 1);
    gridButtons->addWidget(btnSave, 0, 2);
    gridButtons->addWidget(btnSaveAs, 0, 3);
    gridButtons->addWidget(btnBot, 0, 4);
    gridButtons->addWidget(btnPasses, 0, 5);
    gridButtons->addWidget(btnGenerate, 0, 6);
    gridButtons->addWidget(btnTop, 0, 7);
    gridButtons->addWidget(btn8, 0, 8);
    gridButtons->addWidget(btn9, 0, 9);
    gridButtons->addWidget(btn10, 0, 10);
    gridButtons->addWidget(btn11, 0, 11);
    gridButtons->addWidget(btn12, 0, 12);

    gridButtons->addWidget(btnHelp, 0, 13);
#endif

    connect(btnHome, &QPushButton::clicked, [&]() { emit homePageClicked(); });    
    connect(btnBot, &QPushButton::clicked, this, &FormContour::on_btnBot_clicked);

    connect(btnPasses, &QPushButton::clicked, [&]() { emit passesPageClicked(); });
    connect(btnGenerate, &QPushButton::clicked, this, &FormContour::onGenerate);
    connect(btnHelp, &QPushButton::clicked, this, [&](){ emit helpPageClicked(help_file); });
}

void FormContour::createGridView() {
    gridView = new QGridLayout;

    lineContour = new QLineEdit;
    lineContours = new QLineEdit;

    viewContours = new QTableView;
    viewContour = new QTableView;

    ContourTableModel* model = new ContourTableModel();

    viewContours->setModel(new ContoursModel());
    viewContours->horizontalHeader()->hide();

    viewContour->setModel(model);

    groupContours = new QGroupBox(tr("Contours"));
    groupContour = new QGroupBox(tr("Current"));

    groupContours->setLayout(new QVBoxLayout());
    groupContours->layout()->addWidget(lineContours);
    groupContours->layout()->addWidget(viewContours);

    groupContour->setLayout(new QVBoxLayout());
    groupContour->layout()->addWidget(lineContour);
    groupContour->layout()->addWidget(viewContour);

    createViewControl();

    gridView->addWidget(groupContours, 0, 0);
    gridView->addWidget(groupContour, 0, 1);
    gridView->addLayout(vLayoutLeft, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    gridView->addLayout(vLayoutRight, 1, 1, Qt::AlignLeft | Qt::AlignTop);


#if defined(STONE)

#else
    gridView->setColumnStretch(0, 1);
    gridView->setColumnStretch(1, 2);
#endif

    viewContours->show();
    viewContour->show();

//    void pressed(const QModelIndex &index);
//    void clicked(const QModelIndex &index);
//    void doubleClicked(const QModelIndex &index);

//    void activated(const QModelIndex &index);
//    void entered(const QModelIndex &index);

    connect(viewContours, &QTableView::clicked, this, &FormContour::onViewContoursClicked);
    connect(viewContours, &QTableView::activated, this, &FormContour::onViewContoursClicked);

    connect(viewContour, &QTableView::clicked, this, &FormContour::onViewContourClicked);
    connect(viewContour, &QTableView::activated, this, &FormContour::onViewContourClicked);
}

void FormContour::createViewControl() {
    vLayoutLeft = new QVBoxLayout();

    btnNewContour = new QPushButton(tr("New Contour"));
    btnNewContour->setEnabled(false);

    btnAddCutline = new QPushButton(tr("Add Cutline"));
    btnAddCutline->setStatusTip(tr("Add new cutline before the first segment"));

    btnDelete = new QPushButton(tr("Delete"));
    btnDelete->setStatusTip(tr("Delete selected contour"));

    btnClear = new QPushButton(tr("Clear"));
    btnClear->setStatusTip(tr("Clear all contours"));

    vLayoutLeft->addWidget(btnNewContour);
    vLayoutLeft->addWidget(btnAddCutline);
    vLayoutLeft->addWidget(btnDelete);
    vLayoutLeft->addWidget(btnClear);
    vLayoutLeft->setSizeConstraint(QLayout::SetFixedSize);

    vLayoutRight = new QVBoxLayout();

    btnBegin = new QPushButton(tr("First"));
    btnBegin->setStatusTip(tr("Set as the first segment in the contour"));

    btnChangeDir = new QPushButton(tr("Change direction"));
    btnChangeDir->setStatusTip(tr("Change contour direction"));

    btnEdit = new QPushButton();
    btnEdit->setText(tr("Edit"));

    menuEdit = new QMenu;

    actCutline = new QAction(tr("Use as Cutline"));
    actCutline->setStatusTip(tr("Use segment as cutline"));

    actExitPoint = new QAction(tr("Mark as the last segment"));
    actExitPoint->setEnabled(false);

    actRotate = new QAction(tr("Rotate"));
    actRotate->setStatusTip(tr("Rotate contour"));

    actFlipLeftRight = new QAction(tr("Flip Left-Right"));
    actFlipLeftRight->setStatusTip(tr("Flip contour Left-Right"));

    actFlipUpDown = new QAction(tr("Flip Up-Down"));
    actFlipUpDown->setStatusTip(tr("Flip contour Up-Down"));

    actResize = new QAction(tr("Resize"));
    actResize->setStatusTip(tr("Resize contour"));

    menuEdit->addAction(actCutline);
    menuEdit->addAction(actExitPoint);
    menuEdit->addAction(actRotate);
    menuEdit->addAction(actFlipLeftRight);
    menuEdit->addAction(actFlipUpDown);
    menuEdit->addAction(actResize);

    btnEdit->setMenu(menuEdit);

    vLayoutRight->addWidget(btnBegin);
    vLayoutRight->addWidget(btnChangeDir);
    vLayoutRight->addWidget(btnEdit);

    connect(btnNewContour, &QPushButton::clicked, this, &FormContour::on_btnNewContour_clicked);
    connect(btnAddCutline, &QPushButton::clicked, this, &FormContour::on_btnNewCutline_clicked);
    connect(btnDelete, &QPushButton::clicked, this, &FormContour::on_btnDelete_clicked);
    connect(btnClear, &QPushButton::clicked, this, &FormContour::on_btnClear_clicked);

    connect(btnBegin, &QPushButton::clicked, this, &FormContour::onBeginClicked);
    connect(btnChangeDir, &QPushButton::clicked, this, &FormContour::on_btnReverse_clicked);

    connect(actCutline, &QAction::triggered, this, &FormContour::on_actCutline_triggered);
    connect(actExitPoint, &QAction::triggered, this, &FormContour::on_actExitPoint_triggered);
    connect(actRotate, &QAction::triggered, this, &FormContour::on_actRotate_triggered);
    connect(actFlipLeftRight, &QAction::triggered, this, &FormContour::on_actFlipLeftRight_triggered);
    connect(actFlipUpDown, &QAction::triggered, this, &FormContour::on_actFlipUpDown_triggered);
    connect(actResize, &QAction::triggered, this, &FormContour::on_actResize_triggered);
}

FormContour::~FormContour() {}

void FormContour::setFontPointSize(QWidget *w, int pointSize) {
    if (w) {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }
}

void FormContour::setFontPointSize(int pointSize) {
    for (QPushButton* b: buttons)
        setFontPointSize(b, pointSize);

    setFontPointSize(groupContours, pointSize);
    setFontPointSize(groupContour, pointSize);
}

QString FormContour::getFileName() const{
    return file_open ? par.contourFileName : "";
}

void FormContour::resetFileOpen() { file_open = false; }

void FormContour::init() {
    if (m_contour_num >= par.contours.count()) m_contour_num = 0;
    m_row = 0;
    m_column = 0;

    plot();
    initView();
}

bool FormContour::openDxfFileDialog(QFile& file, QString& fileDir, QString& fileName) {
    QString fullFileName = fileDir + "/" + fileName;

    if (!QFile::exists(fullFileName)) {
        fileName.clear();

        if (!QDir(fileDir).exists())
            fileDir = QDir::homePath();
    }

    QFileDialog dialog(this, tr("Open DXF file"), fileDir + "/" + fileName,
                       tr("DXF files") + " (*.dxf *.DXF)");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    dialog.setOption(QFileDialog::ReadOnly);

//    dialog.setWindowModality(Qt::ApplicationModal);
//    dialog.setModal(true);
//    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);

//    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (dialog.exec()) {
        QStringList list = dialog.selectedFiles();
        fileDir = dialog.directory().canonicalPath();
        file.setFileName(list[0]);

        QFileInfo fInfo(file);
        fileName = fInfo.fileName();
        return true;
    }

    return false;
}

void FormContour::on_btnBot_clicked() {
    QFile file;

    file_open = false;
    emit fileNameChanged("");

    if (openDxfFileDialog(file, par.fileDir, par.contourFileName)) {
        Dxf bot;
        bot.init(par.fileDir.toStdString(), par.contourFileName.toStdString());

        if (bot.parse()) {
            txtMsg->setText(tr("Bottom layer loaded successfully") + "\n");

            file_open = true;
            emit fileNameChanged(par.contourFileName);

            Dxf free, unused;

            bool OK = bot.sort(free, unused);

            if (!free.empty()) {
                txtMsg->setText(tr("Removed unconnected segments from DXF") + ":\n");

                for (list<DxfEntity*>::const_iterator it = free.entities().cbegin(); it != free.entities().cend(); ++it)
                    if (*it)
                        txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
                    else
                        txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (1): DxfEntity is NULL\n");
            }

            if (!unused.empty()) {
                txtMsg->setText(tr("Removed extra tails from DXF") + ":\n");

                for (list<DxfEntity*>::const_iterator it = unused.entities().cbegin(); it != unused.entities().cend(); ++it)
                    if (*it)
                        txtMsg->setText(txtMsg->toPlainText() + (*it)->toString().c_str() + "\n");
                    else
                        txtMsg->setText(txtMsg->toPlainText() + "Error at void FormContour::on_btnBot_clicked() (2): DxfEntity is NULL\n");
            }

            if (!OK)
                txtMsg->setText(txtMsg->toPlainText() + tr("Bottom layer DXF segments sorting error") + ": " + bot.lastError().c_str() + "\n");

            par.contours.new_back();
            par.contours.back()->setBot(bot);

            init();
        }
        else {
            txtMsg->setText(tr("Bottom layer DXF parsing error") + ": " + QString(bot.lastError().c_str()));
            plot(bot);
        }

        par.saveSettings();
    }
}

void FormContour::on_btnTop_clicked() {

}

void FormContour::on_btnNewContour_clicked() {
    par.contours.new_back();
    initView();
}

void FormContour::on_btnNewCutline_clicked() {
    if (!par.contours.empty()) {
        const ContourPair* pair = par.contours.at(m_contour_num);
        const fpoint_valid_t entry_pt = pair && pair->bot() ? pair->bot()->first_point() : fpoint_valid_t(false);

        if (pair && pair->type() == CONTOUR_TYPE::CONTOUR_MAIN && entry_pt.valid) {
            bool insert_before = m_contour_num == 0 || par.contours.at(m_contour_num - 1)->type() == CONTOUR_TYPE::CONTOUR_MAIN;

            const fpoint_valid_t exit_pt = pair && pair->bot() ? pair->bot()->last_point() : fpoint_valid_t(false);
            bool insert_after = !insert_before && exit_pt.valid;

            if (insert_before || insert_after) {
                NewCutlineDialog* dialog = new NewCutlineDialog(this);

                if (!dialog)
                    return;

                if (insert_before) {
                    dialog->set(0, entry_pt.x, 0, entry_pt.y);
                    dialog->enable(0x5);
                }
                else if (insert_after) {
                    dialog->set(exit_pt.x, 0, exit_pt.y, 0);
                    dialog->enable(0xA);
                }

                dialog->exec();

                if (dialog->result() == QDialog::Accepted) {
                    double x0, x1, y0, y1;
                    dialog->get(x0, x1, y0, y1);

                    DxfLine line;
                    if (dialog->isRel()) {
                        if (insert_after) {
                            line.setX0(entry_pt.x);
                            line.setX1(entry_pt.x + x1);
                            line.setY0(entry_pt.y);
                            line.setY1(entry_pt.y + y1);
                        }
                        else { // before
                            line.setX0(entry_pt.x + x0);
                            line.setX1(entry_pt.x);
                            line.setY0(entry_pt.y + y0);
                            line.setY1(entry_pt.y);
                        }
                    }
                    else {
                        line.setX0(x0);
                        line.setX1(x1);
                        line.setY0(y0);
                        line.setY1(y1);
                    }

                    qDebug() << "Dialog OK";

                    ContourPair incut(CONTOUR_TYPE::CONTOUR_CUTLINE);
                    if (incut.bot())
                        incut.bot()->push_back(line);

                    if (insert_before) {
                        ContourPair outcut(incut);
                        outcut.reverse();

                        par.contours.push_front(incut);
                        par.contours.push_back(outcut);

                        m_row = 0;
                        m_column = 0;
                        par.contours.clearSelected();
                    }
                    else {
                        par.contours.insert_after(m_contour_num, incut);
                        m_contour_num++;
                    }

                    init();

                    viewContours->selectRow(int(m_contour_num));
                    viewContour->selectRow(0);
                    par.contours.select(m_contour_num, 0, 0);
                    plot();
                }

                delete dialog;
                dialog = nullptr;
            }
        }
    }
}

void FormContour::on_btnDelete_clicked() {
    par.contours.remove(m_contour_num);
    init();
}

void FormContour::on_btnClear_clicked() {
    qDebug() << "Clear all contours:";
    qDebug() << par.contours.toString().c_str();

    setEmptyModel();

    par.contours.clear();

    qDebug() << par.contours.toString().c_str();

    init();
}

void FormContour::on_btnReverse_clicked() {
    if (m_contour_num < par.contours.count()) {
        ContourPair* pair = par.contours.at(m_contour_num);
        if (pair) pair->reverse();
        par.contours.clearSelected();
        init();

        viewContour->selectRow(0);
        par.contours.select(m_contour_num, 0, 0);
        plot();
    }
}

void FormContour::onBeginClicked() {
    if (m_contour_num < par.contours.count()) {
        ContourPair* pair = par.contours.at(m_contour_num);
        if (pair) {
            qDebug() << pair->toStringShort().c_str();
            pair->setFirst(m_column, m_row);
            qDebug() << pair->toStringShort().c_str();
        }
        else
            qDebug() << "Error: No pair";

        init();

        viewContours->selectRow(int(m_contour_num));
        viewContour->selectRow(0);
    }
}

void FormContour::onViewContoursClicked(const QModelIndex &index) {
    if (index.isValid()) {
        m_contour_num = index.row() < 0 ? 0 : size_t(index.row());
        if (m_contour_num >= par.contours.count()) m_contour_num = par.contours.count() - 1;

        initContourView();

        const ContourPair* pair = par.contours.at(m_contour_num);

        if (pair && !pair->empty())
            if (pair->type() == CONTOUR_TYPE::CONTOUR_MAIN)
                lineContours->setText(QString("%1 Contour (%2)").arg(m_contour_num + 1).arg(pair->count()));
            else
                lineContours->setText(QString("%1 Cutline (%2)").arg(m_contour_num + 1).arg(pair->count()));
        else
            lineContours->clear();

        txtMsg->setText(par.contours.toString().c_str());

//        viewContours->selectRow(int(m_contour_num));
        viewContour->selectRow(0);
        par.contours.select(m_contour_num, 0, 0);
        plot();
    }
}

void FormContour::onViewContourClicked(const QModelIndex& index) {
    if (index.isValid()) {
        m_row = index.row() < 0 ? 0 : size_t(index.row());
        m_column = index.column() < 0 ? 0 : size_t(index.column());

        const ContourPair* const pair = par.contours.at(m_contour_num);

        const Dxf* const bot = pair ? pair->bot() : nullptr;

        switch (m_column) {
        case 0:
            if (bot && m_row < bot->count()) {
                lineContour->setText(QString("%1 Line XY").arg(m_row + 1));
                txtMsg->setText(bot->at(m_row)->toString().c_str());
                par.contours.select(m_contour_num, m_row, m_column);
                plot();
            }

            break;
        case 1:

            break;
        }
    }
}

// only for single contour
void FormContour::on_actCutline_triggered() {
//    if (m_contour_num == 0 && m_contour_num < par.contours.count()) {
    if (par.contours.count() == 1) {
        ContourPair incut(CONTOUR_TYPE::CONTOUR_CUTLINE);
        const ContourPair* const pair = par.contours.front();
//        ContourPair* pair = par.contours.at(m_contour_num);

        if (pair && !pair->empty()) {
            if (!pair->emptyBot() && !pair->bot()->isLoop()) {
                if (m_row == 0)
                    incut.setBot( Dxf(pair->bot()->cut_front()) );
                else if (m_row == pair->countBot() - 1) {
                    incut.setBot( Dxf(pair->bot()->cut_back()) );
                    incut.bot()->reverse();
                }
            }

            if (!pair->emptyTop() && !pair->top()->isLoop()) {
                if (m_row == 0)
                    incut.setTop( Dxf(pair->top()->cut_front()) );
                else if (m_row == pair->countTop() - 1) {
                    incut.setTop( Dxf(pair->top()->cut_back()) );
                    incut.top()->reverse();
                }
            }

            if (!incut.emptyBot() || !incut.emptyTop()) {
                ContourPair outcut(incut);
                outcut.reverse();

//                par.contours.insert(m_contour_num, incut);
                par.contours.push_front(incut);
                par.contours.push_back(outcut);

                m_row = 0;
                m_column = 0;
                par.contours.clearSelected();
                init();
            }
        }
    }
    else {
        txtMsg->setText("It works only for single unloop contour");
    }
}

void FormContour::on_actExitPoint_triggered() {

}

void FormContour::on_actRotate_triggered() {
    if (par.contours.empty())
        return;

    RotateDialog* dialog = new RotateDialog(this);
    if (!dialog) return;

    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        double angle = dialog->value();

        int data = ProgramParam::swapXY << 2 | ProgramParam::reverseX << 1 | ProgramParam::reverseY;

        if (data == 0b010 || data == 0b001 || data == 0b100 || data == 0b111)
            angle = -angle;

        qDebug() << "Dialog OK" << "Rotate: " << angle;
        par.contours.rotate(M_PI / 180 * angle);
        init();
        plot();
    }

    delete dialog;
}

void FormContour::on_actFlipLeftRight_triggered() {
    if (par.contours.empty()) return;

    ProgramParam::swapXY ? par.contours.flipY() : par.contours.flipX();
    init();
    plot();
}

void FormContour::on_actFlipUpDown_triggered() {
    if (par.contours.empty()) return;
    ProgramParam::swapXY ? par.contours.flipX() : par.contours.flipY();
    init();
    plot();
}

void FormContour::on_actResize_triggered() {
    if (par.contours.empty() || m_contour_num >= par.contours.count())
        return;

    ContourPair* pair = par.contours.at(m_contour_num);

    if (!pair || pair->type() != CONTOUR_TYPE::CONTOUR_MAIN)
        return;

    ContourRange range = pair->range();
    center = fpoint_valid_t(range.center(), center.valid);

    ResizeDialog* dialog = new ResizeDialog(range.width(), range.height(), center, this);
    if (!dialog) return;

    dialog->exec();

    double pct = 1;

    if (dialog->result() == QDialog::Accepted) {
        if (dialog->isPct()) {
            pct = dialog->pct() / 100;
            qDebug() << "Dialog OK" << "Rotate pct: " << pct;
        }
        else if (dialog->isRect()) {
            qDebug() << "Dialog OK" << "Resize Width: " << dialog->rectWidth() << "Height: " << dialog->rectHeight();
            frect_t rect(fpoint_t(range.x_min, range.y_min), fpoint_t(range.x_max, range.y_max));
            pct = rect.resize(dialog->rectWidth(), dialog->rectHeight());
        }
        else if (dialog->isRatio()) {
            pct = dialog->ratioNewSize() / dialog->ratioBaseSize();
            qDebug() << "Dialog OK" << "Ratio Old: " << dialog->ratioBaseSize() << "New: " << dialog->ratioNewSize() << "(" << pct << ")";
        }
        else
            qDebug() << "Dialog Error";


        center = dialog->center();
        fpoint_t base(center.valid ? center.x : range.x_min, center.valid ? center.y : range.y_min);

        pair->scale(pct, base);

        if (m_contour_num) {
            ContourPair* incut = par.contours.at(m_contour_num - 1);

            if (incut && incut->type() == CONTOUR_TYPE::CONTOUR_CUTLINE) {
                if (!incut->emptyBot())
                    incut->bot()->back()->change_1( pair->firstBot() );

                if (!incut->emptyTop())
                    incut->top()->back()->change_1( pair->firstTop() );
            }
        }

        if ((m_contour_num + 1) < par.contours.count()) {
            ContourPair* outcut = par.contours.at(m_contour_num + 1);

            if (outcut && outcut->type() == CONTOUR_TYPE::CONTOUR_CUTLINE) {
                if (!outcut->emptyBot())
                    outcut->bot()->front()->change_0( pair->lastBot() );

                if (!outcut->emptyTop())
                    outcut->top()->front()->change_0( pair->lastTop() );
            }
        }

        init();
        plot();
    }

    delete dialog;
}

void FormContour::onGenerate() {
    // for loop with incut automatic add outcut
    const size_t count = par.contours.count();

    if (count == 0) {
        txtMsg->setText(tr("No contour"));
        return;
    }

    const bool isCutLine = par.contours.at(0)->type() == CONTOUR_TYPE::CONTOUR_CUTLINE;

    if (count == 1 && isCutLine) {
        txtMsg->setText(tr("No contour. Only cutline"));
        return;
    }

    const bool isLoop = count > 2 && par.contours.at(1)->isLoop();
    const bool hasOut = count > 2 && par.contours.at(1)->hasOut();

    if (count == 2 && isCutLine && isLoop && !hasOut) {
        ContourPair* incut = par.contours.at(0);
        ContourPair outcut(*incut);
        outcut.reverse();
        par.contours.push_back(std::move(outcut));
    }
    // todo: add empty lines in front and in back
//    else if (count == 1 && !isCutLine) { // todo: G-code without passes
//        ContourPair incut(CONTOUR_TYPE::CONTOUR_CUTLINE);
//        ContourPair outcut(CONTOUR_TYPE::CONTOUR_CUTLINE);
//        const ContourPair* const pair = par.contours.front();

//        if (pair && !pair->empty()) {
//            if (!pair->emptyBot()) {
//                incut.setBot( Dxf(pair->bot().cut_front()) );
//                outcut.setBot( Dxf(pair->bot().cut_back()) );
//            }

//            if (!pair->emptyTop()) {
//                incut.setTop( Dxf(pair->top().cut_front()) );
//                outcut.setTop( Dxf(pair->top().cut_back()) );
//            }

//            if ((!incut.emptyBot() || !incut.emptyTop()) && (!outcut.emptyBot() || !outcut.emptyTop())) {
//                ContourPair outcut(incut);
//                outcut.reverse();

////                par.contours.insert(m_contour_num, incut);
//                par.contours.push_front(incut);
//                par.contours.push_back(outcut);

//                m_row = 0;
//                m_column = 0;
//                par.contours.clearSelected();
//                init();
//            }
//        }
//    }
//    else {
//        txtMsg->setText(tr("No cutline"));
//    }

    par.contours.shift(); // to (0,0)
    par.contours.setCut(par.cut, par.genModesList);

//    if (count == 1) {
//        deque<ContourPair> contours = {*par.contours.front()};
//
//        if (par.gcode.generate( par.cut, contours )) {
//            par.gcodeText = par.gcode.toText().c_str();
//            emit editPageClicked(true);
//        }
//        else
//            txtMsg->setText(par.gcode.lastError().c_str());
//    }
//    else {
//        ContourPass pass(par.contours);
//        pass.generate();

//        if (par.gcode.generate( par.cut, pass.contours() )) {
//            par.gcodeText = par.gcode.toText().c_str();
//            emit editPageClicked(true);
//        }
//        else
//            txtMsg->setText(par.gcode.lastError().c_str());
//    }

    ContourPass pass(par.contours);
    pass.generate();

    if (par.gcode.generate( par.cut, pass.contours() )) {
        par.gcodeText = par.gcode.toText().c_str();
        emit editPageClicked(true);
    }
    else
        txtMsg->setText(par.gcode.lastError().c_str());
}

void FormContour::initView() {
    qDebug() << "Init. " << "Contours: " << par.contours.count();

    QItemSelectionModel* m = viewContours->selectionModel();
    viewContours->setModel(new ContoursModel(&par.contours));
    delete m;

    viewContours->resizeColumnsToContents();
    lineContours->clear();

    initContourView();
}

void FormContour::setEmptyModel() {
    QItemSelectionModel* m = viewContour->selectionModel();
    viewContour->setModel(new ContourTableModel());
    delete m;
}

void FormContour::initContourView() {
    qDebug() << "Init. " << "Contours: " << par.contours.count();

    QItemSelectionModel* m = viewContour->selectionModel();
    viewContour->setModel(new ContourTableModel(par.contours.at(m_contour_num)));
    delete m;

    viewContour->resizeColumnsToContents();
    lineContour->clear();
}

void FormContour::plot() {
//    plotView.plot(par.contours, plotViewSize());
    plotView.setSwapXY(par.swapXY);
    plotView.setInverseX(par.reverseX);
    plotView.setInverseY(par.reverseY);
    plotView.setShowXY(par.showXY);
    plotView.plot(par.contours);

    if (!par.contours.empty() && !par.contours.front()->emptyBot()) {
        fpoint_t pt = par.contours.front()->firstBot();
        plotView.addSymbolSquare(&pt);
    }
}

void FormContour::plot(const Dxf& contour) {
    plotView.setSwapXY(par.swapXY);
    plotView.setInverseX(par.reverseX);
    plotView.setInverseY(par.reverseY);
    plotView.setShowXY(par.showXY);
    plotView.plot(contour);
}

QSize FormContour::plotViewSize() const {
    return plotViewSize(size());
}

QSize FormContour::plotViewSize(const QSize& formSize) const {
    QSize btnSize = gridButtons->sizeHint();
    QSize txtSize = txtMsg->size();

    int w = int(formSize.width() * m_plot_view_pct);
    int h = formSize.height() - btnSize.height() - txtSize.height();

    qDebug() << "Widget size: " << formSize << ", plot width:" << w << ", msg height: " << txtSize.height() << ", buttons height: " << btnSize.height();

    return QSize(w, h);
}

void FormContour::resizeEvent(QResizeEvent* event) {
    if (event && !plotView.onResizeEvent( plotViewSize(event->size()) ))
        event->ignore();
//    updateGeometry();
//    update();
}
