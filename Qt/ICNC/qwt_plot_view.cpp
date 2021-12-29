#include "qwt_plot_view.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_layout.h>

using namespace std;

QVector<QPointF> QwtPlotView::m_pts;

QwtPlotView::QwtPlotView() :
    margin(40),
    owner(true),
    qwtPlot(new QwtPlot),
    grid(nullptr),
    picker(nullptr),
    m_range(ContourRange(0, 100, 0, 100))
{
    if (qwtPlot)
        qwtPlot->setMinimumSize(200, 200);

    emptyPlot();
}

QwtPlotView::QwtPlotView(const QwtPlotView& other) :
    margin(other.margin),
    owner(true),
    qwtPlot(new QwtPlot(other.qwtPlot)),
    m_range(other.m_range)
{
    if (qwtPlot)
        qwtPlot->setMinimumSize(QSize(200, 200));
}

QwtPlotView::QwtPlotView(QwtPlotView&& other) :
    margin(other.margin),
    owner(other.owner),
    qwtPlot(other.qwtPlot),
    m_range(other.m_range)
{
    if (qwtPlot)
        qwtPlot->setMinimumSize(200, 200);

    other.owner = false;
    other.qwtPlot = nullptr;
}

QwtPlotView::~QwtPlotView() {
    if (owner && qwtPlot) {
        delete qwtPlot;
        qwtPlot = nullptr;
    }
}

QwtPlot* QwtPlotView::widget() {
    owner = false;
    return qwtPlot;
}

void QwtPlotView::emptyPlot(const QSize& frameSize) {
    if (!qwtPlot) return;

//    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    sp.setHeightForWidth(true);
    qwtPlot->setSizePolicy(sp);

    qwtPlot->detachItems();
    qwtPlot->setCanvasBackground(Qt::white);

    m_range = ContourRange(0, 100, 0, 100);
    setAxis(frameSize);

    qwtPlot->replot();
}

void QwtPlotView::emptyPlot() {
    if (qwtPlot)
        emptyPlot(qwtPlot->size());
}

void QwtPlotView::plot(const ContourList& contourList, const QSize& frameSize) {
    ContourRange range;
    QVector<QPointF> pts_bot, pts_top;

    if (!qwtPlot) return;

    qwtPlot->detachItems(QwtPlotItem::Rtti_PlotItem, true); // it's default

    int i = 0, j = 0, k = 0;
    bool passed_bot_reg = true, passed_top_reg = true;

    for (const ContourPair& pair: contourList.contours()) {
        for (const DxfEntity* ent: pair.bot()->entities()) {
            bool passed = i < contourList.currentContourNumber() || (i == contourList.currentContourNumber() && j <= contourList.currentSegmentNumber());

            if (passed ^ passed_bot_reg) {
                if (!pts_bot.empty()) {
                    addPlot(qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
                    pts_bot.clear();
                }
                passed_bot_reg = passed;
            }

            if (ent)
                copy_back(pts_bot, ent->getPoints());

            j++;
        }

        for (const DxfEntity* ent: pair.top()->entities()) {
            bool passed = i < contourList.currentContourNumber() || (i == contourList.currentContourNumber() && k <= contourList.currentSegmentNumber());

            if (passed ^ passed_top_reg) {
                if (!pts_top.empty()) {
                    addPlot(qwtPlot, range, pts_top, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkBlue);
                    pts_top.clear();
                }
                passed_top_reg = passed;
            }

            if (ent)
                copy_back(pts_top, ent->getPoints());

            k++;
        }

        i++;
    }

    if (!pts_bot.empty()) {
        addPlot(qwtPlot, range, pts_bot, m_swapXY, passed_bot_reg ? Qt::GlobalColor::red : Qt::GlobalColor::blue);
        pts_bot.clear();
    }

    if (!pts_top.empty()) {
        addPlot(qwtPlot, range, pts_top, m_swapXY, passed_top_reg ? Qt::GlobalColor::darkRed : Qt::GlobalColor::darkBlue, 2);
        pts_top.clear();
    }

    addPlot(qwtPlot, range, contourList.selectedBot(), m_swapXY);
    addPlot(qwtPlot, range, contourList.selectedTop(), m_swapXY);

//    fpoint_t pt(0,0);
//    range.append({pt});
//    addPoint(newChart, &pt, nullptr);

    addPoint(qwtPlot, contourList.botPos(), contourList.topPos(), m_swapXY);
    addLine(qwtPlot, contourList.botPos(), contourList.topPos(), m_swapXY);

    tweakPlot(qwtPlot, range, frameSize);
}

void QwtPlotView::plot(const ContourList& contourList) {
    if (qwtPlot)
        plot(contourList, qwtPlot->size());
}

void QwtPlotView::plot(const Dxf& ctr, const QSize& frameSize) {
    ContourRange range;
    QVector<QPointF> pts;

    if (!qwtPlot) return;
    qwtPlot->detachItems();

    if (ctr.empty()) return;

    copy_back(pts, ctr.getPoints());
    addPlot(qwtPlot, range, pts, m_swapXY);
    tweakPlot(qwtPlot, range, frameSize);
}

void QwtPlotView::plot(const Dxf& ctr) {
    if (qwtPlot)
        plot(ctr, qwtPlot->size());
}

void QwtPlotView::plot(const QVector<QPointF> &pts, ContourRange& range, const QSize &frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    qwtPlot->detachItems();

    if (pts.empty()) return;

    addPlot(qwtPlot, range, pts, m_swapXY, color, width, style);

    const QPointF& pt = pts.at(pts.count() - 1);
    fpoint_t last_pt(pt.x(), pt.y());

    addPoint(qwtPlot, &last_pt, nullptr, m_swapXY, tipColor);

    tweakPlot(qwtPlot, range, frameSize);
}

void QwtPlotView::plot(const QVector<QPointF> &pts, ContourRange& range, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    if (qwtPlot)
        plot(pts, range, qwtPlot->size(), color, tipColor, width, style);
}

void QwtPlotView::plot(const QVector<QPointF> &pts, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
    if (qwtPlot) {
        ContourRange range;
        plot(pts, range, qwtPlot->size(), color, tipColor, width, style);
    }
}

//void QwtPlotView::plot(const QVector<QPointF> &pts, const QSize &frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style) {
//    ContourRange range;
//    plot(pts, range, frameSize, color, tipColor, width, style);
//}

void QwtPlotView::tweakPlot(QwtPlot* const plot, const ContourRange& range, const QSize& frameSize) {
    static QSize frameSize_old = {0, 0};

    if (!qwtPlot) return;

    bool print_new = frameSize != frameSize_old || !range.equal(m_range);
    frameSize_old = frameSize;

    if (plot && !plot->itemList().empty()) {
        qwtPlot->setCanvasBackground( Qt::white );

        grid = new QwtPlotGrid();
        grid->setMajorPen(QPen( Qt::gray, 1 ));
        grid->setMinorPen(Qt::black, 1, Qt::PenStyle::DotLine);
        grid->enableXMin(true);
        grid->enableYMin(true);
        grid->attach(qwtPlot);

        // todo: delete picker
//        QwtPlotPicker* picker = new QwtPlotPicker(
//            QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, qwtPlot->canvas()
//        );
//        picker->setRubberBandPen( QColor(Qt::red) );
//        picker->setTrackerPen( QColor(Qt::black) );
//        picker->setStateMachine( new QwtPickerDragPointMachine );

        m_range = range;

        if (print_new) qDebug() << "Plot items: " << plot->itemList().count();

        setAxis(frameSize);

        plot->updateGeometry();
        plot->update();
        plot->show();

        if (print_new) qDebug() << "Plot size: " << plot->size();
    }
    else
        emptyPlot(frameSize);

    QwtScaleEngine* engine = plot->axisScaleEngine(QwtPlot::yLeft);
    if (engine)
        engine->setAttribute(QwtScaleEngine::Attribute::Inverted, true);
//    qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, );

    if (plot)
        plot->replot();
}

void QwtPlotView::setAxis(const QSize& frameSize) {
    static ContourRange range_old;
    static QSize frameSize_old = {0, 0};

    if (!qwtPlot) return;

    if (!m_range.valid || frameSize.width() == 0 || frameSize.height() == 0) return;

    bool print_new = !m_range.equal(range_old) || frameSize != frameSize_old;
    range_old = m_range;
    frameSize_old = frameSize;

    double width = m_range.width();
    double height = m_range.height();

    if (print_new) qDebug() << "Aspect ratio " << frameSize.width() << ":" << frameSize.height();
    if (print_new) qDebug() << "Plot ratio " << width << ":" << height;

    double aspect_ratio = static_cast<double>(frameSize.width() - D_WIRE_PX) / static_cast<double>(frameSize.height() - D_WIRE_PX);

    double scale; // mm / pixel

    if (width < 0.001 && height < 0.001)
        scale = 1;
    else {
        double plot_ratio = width / height;

        if (plot_ratio > aspect_ratio)
            scale = width / (frameSize.width() - D_WIRE_PX);
        else
            scale = height / (frameSize.height() - D_WIRE_PX);
    }

    if (print_new) qDebug() << "Scale " << scale << " mm / pixel";

    width = frameSize.width() * scale; // mm
    height = frameSize.height() * scale; // mm
    double wire_mm = D_WIRE_PX * scale;

    if (print_new) qDebug() << "New plot ratio " << width << ":" << height;

//    double x_min = m_range.x_min - wire_mm / 2;
//    double x_max = m_range.x_min + width + wire_mm / 2;
//    double y_min = m_range.y_min - wire_mm / 2;
//    double y_max = m_range.y_min + height + wire_mm / 2;

    double x_mean = m_range.meanX();
    double x_delta = (width + wire_mm) / 2;
    double x_min = x_mean - x_delta;
    double x_max = x_mean + x_delta;

    double y_mean = m_range.meanY();
    double y_delta = (height + wire_mm) / 2;
    double y_min = y_mean - y_delta;
    double y_max = y_mean + y_delta;

    double x_step = calcPlotStep(x_min, x_max, 10);
    double y_step = calcPlotStep(y_min, y_max, 10);
    double step = x_step > y_step ? x_step : y_step;

    if (m_inverseX)
        qwtPlot->setAxisScale(QwtPlot::xBottom, x_max, x_min, -step);
    else
        qwtPlot->setAxisScale(QwtPlot::xBottom, x_min, x_max, step);

    if (m_inverseY)
        qwtPlot->setAxisScale(QwtPlot::yLeft, y_max, y_min, -step);
    else
        qwtPlot->setAxisScale(QwtPlot::yLeft, y_min, y_max, step);

    if (m_showXY) {
        qwtPlot->setAxisTitle(QwtPlot::Axis::xBottom, m_swapXY ? "Y" : "X");
        qwtPlot->setAxisTitle(QwtPlot::Axis::yLeft, m_swapXY ? "X" : "Y");
    }

    qwtPlot->updateAxes();

//    QwtPlotRescaler* rescaler = new QwtPlotRescaler( qwtPlot->canvas() );
//    rescaler->setReferenceAxis(QwtPlot::xBottom);
//    rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
//    rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
//    rescaler->setEnabled(true);
//    rescaler->rescale();
//    qwtPlot->plotLayout()->setAlignCanvasToScales(true);

//    qwtPlot->replot();
}

void QwtPlotView::swapXY(QVector<QPointF>& pts) {
    for (QPointF& pt: pts)
        pt = QPointF(pt.y(), pt.x());
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, const QVector<QPointF>& pts, bool _swapXY, const QColor& color, int width, Qt::PenStyle style) {
    if (!newPlot || pts.empty()) return;

    m_pts = pts;

    if (_swapXY)
        swapXY(m_pts);

    newRange.append(m_pts);

    QwtPlotCurve* curve = new QwtPlotCurve;
    if (curve) {
        curve->setPen(color, width, style);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

        curve->setSamples(m_pts);

        curve->attach(newPlot);
    }
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, const Dxf* const dxf, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    QVector<QPointF> pts;

    if (newPlot && dxf && !dxf->empty()) {
        copy_back(pts, dxf->getPoints());
        addPlot(newPlot, newRange, pts, swapXY, color, width, style);
    }
}

void QwtPlotView::addPlot(QwtPlot* const newPlot, ContourRange& newRange, const DxfEntity* const entity, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    QVector<QPointF> pts;

    if (newPlot && entity) {
        copy_back(pts, entity->getPoints());
        addPlot(newPlot, newRange, pts, swapXY, color, width, style);
    }
}

void QwtPlotView::addPoint(QwtPlot* const newPlot, const fpoint_t* const bot, const fpoint_t* const top, bool swapXY, const QColor& botColor, const QColor& topColor, int size) {
    if (!newPlot) return;

    int width = size / 2;

    if (bot) {
        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
            curve->setPen(botColor, width);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Style::Ellipse, QBrush(botColor), QPen(botColor, width), QSize(width, width) );
            curve->setSymbol(symbol);

            curve->setSamples( {swapXY ? QPointF(bot->y, bot->x) : QPointF(bot->x, bot->y)} );
            curve->attach(newPlot);
        }
    }

    if (top) {
        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
            curve->setPen(topColor, width);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Style::Ellipse, QBrush(topColor), QPen(topColor, width), QSize(width, width) );
            curve->setSymbol(symbol);

            curve->setSamples( {swapXY ? QPointF(top->y, top->x) : QPointF(top->x, top->y)} );
            curve->attach(newPlot);
        }
    }
}

void QwtPlotView::addSymbolSquare(const fpoint_t* const bot, const QColor& botColor, int size) {
    if (!qwtPlot) return;

    if (bot) {
        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
    //        curve->setPen(botColor, 1);
            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Style::Rect, QBrush(botColor, Qt::BrushStyle::NoBrush), QPen(botColor, 1), QSize(size, size) );
            curve->setSymbol(symbol);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            curve->setSamples( {m_swapXY ? QPointF(bot->y, bot->x) : QPointF(bot->x, bot->y)} );
            curve->attach(qwtPlot);
        }
    }
}

bool QwtPlotView::isSwapXY() const { return m_swapXY; }
void QwtPlotView::setSwapXY(bool value) { m_swapXY = value; }

bool QwtPlotView::isInverseX() const { return m_inverseX; }
void QwtPlotView::setInverseX(bool value) { m_inverseX = value; }

bool QwtPlotView::isInverseY() const { return m_inverseY; }
void QwtPlotView::setInverseY(bool value) { m_inverseY = value; }

bool QwtPlotView::isShowXY() const { return m_showXY; }
void QwtPlotView::setShowXY(bool value) { m_showXY = value; }

void QwtPlotView::addLine(QwtPlot* const newPlot, const fpoint_t* const bot, const fpoint_t* const top, bool swapXY, const QColor& color, int width, Qt::PenStyle style) {
    if (top && bot && newPlot) {
        QVector<QPointF> pts = {
            swapXY ? QPointF(bot->y, bot->x) : QPointF(bot->x, bot->y),
            swapXY ? QPointF(top->y, top->x) : QPointF(top->x, top->y)
        };

        QwtPlotCurve* curve = new QwtPlotCurve;
        if (curve) {
            curve->setPen(color, width, style);
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

            curve->setSamples(pts);
            curve->attach(newPlot);
        }
    }
}

bool QwtPlotView::onResizeEvent(const QSize& newFrameSize) {
    if (!m_range.valid || !qwtPlot) return false;

    setAxis(newFrameSize);
    qwtPlot->updateGeometry();
    qwtPlot->update();
    return true;
}

void QwtPlotView::copy_back(vector<fpoint_t>& pts, const vector<fpoint_t>& ent_pts) { // to template with float, or uint16_t and scale
    fpoint_t cur;

    for (const fpoint_t& pt: ent_pts) {
        if (pts.empty()) {
            pts.push_back(pt);
            cur = pt;
        }
        else if (pt != cur) {
            pts.push_back(pt);
            cur = pt;
        }
    }
}

void QwtPlotView::copy_back(QVector<QPointF>& pts, const vector<fpoint_t>& ent_pts) {
    fpoint_t cur;

    for (const fpoint_t& pt: ent_pts) {
        if (pts.empty()) {
            pts.push_back(QPointF(pt.x, pt.y));
            cur = pt;
        }
        else if (pt != cur) {
            pts.push_back(QPointF(pt.x, pt.y));
            cur = pt;
        }
    }
}

double QwtPlotView::calcPlotStep(double min, double max, size_t n) {
    double w = (max - min) * 10;
    double step = w / n;

    double e = log10(step);
    e = trunc(e);
    double exp = pow(10, e);

    if (step / exp < 1)
        exp = e < 0 ? exp / 10 : exp * 10;

    step /= exp; // 1..9
    step = ceil(step);
    int step_int = static_cast<int>(step);

    if (step_int == 0)
        return 0; // error
    else if (step_int == 1)
        step = 1;
    else if (step_int == 2)
        step = 2;
    else if (step_int <= 5)
        step = 5;
    else
        step = 10;

    return step * exp / 10;
}
