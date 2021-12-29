#ifndef PLOT_VIEW_H
#define PLOT_VIEW_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

#include "contour_list.h"
#define D_WIRE_PX (15)
#define INPUT_SQUARE_PX (10)

class QwtPlotView {
    const int margin;

    bool owner;
    QwtPlot* qwtPlot;
    QwtPlotGrid* grid;
    QwtPlotPicker* picker;

    ContourRange m_range;
    bool m_swapXY{false}, m_inverseX{false}, m_inverseY{false};
    bool m_showXY{false};

    static QVector<QPointF> m_pts; // buffer

    static void swapXY(QVector<QPointF>& pts);

    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const QVector<QPointF>& pts,
        bool swapXY,
        const QColor& color = Qt::GlobalColor::blue, int width = 2, Qt::PenStyle = Qt::SolidLine
    );
    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const Dxf* const dxf,
        bool swapXY,
        const QColor& color = Qt::GlobalColor::blue, int width = 2, Qt::PenStyle = Qt::SolidLine
    );
    static void addPlot(
        QwtPlot* const newPlot, ContourRange& newRange,
        const DxfEntity* const entity,
        bool swapXY,
        const QColor& color = Qt::GlobalColor::yellow, int width = 4, Qt::PenStyle = Qt::SolidLine
    );
    static void addPoint(
        QwtPlot* const newPlot,
        const fpoint_t* const botPoint, const fpoint_t* const topPoint,
        bool swapXY,
        const QColor& botColor = Qt::GlobalColor::blue,
        const QColor& topColor = Qt::GlobalColor::cyan,
        int size = D_WIRE_PX
    );

    void addLine(
        QwtPlot* const newPlot,
        const fpoint_t* const bot, const fpoint_t* const top,
        bool swapXY,
        const QColor& color = Qt::GlobalColor::blue, int width = 2, Qt::PenStyle = Qt::DotLine
    );

    void tweakPlot(QwtPlot* const newPlot, const ContourRange& range, const QSize& frameSize);
    void setAxis(const QSize& frameSize);
//    void replaceChart(QwtPlot*& newChart);
//    void replaceChart(QwtPlot*& newChart, const ContourRange& range, const QSize& size);

public:
    QwtPlotView();
    QwtPlotView(const QwtPlotView& other);
    QwtPlotView(QwtPlotView&& other);
    ~QwtPlotView();

    QwtPlot* get() const { return qwtPlot; }
    QwtPlot* widget();

    QSize size() const { return qwtPlot ? qwtPlot->size() : QSize(0, 0); }

    void plot(const ContourList& contourList, const QSize& frameSize);
    void plot(const ContourList& contourList);

    void plot(const Dxf& contour, const QSize& frameSize);
    void plot(const Dxf& contour);

    void plot(const QVector<QPointF>& pts, ContourRange& range, const QSize& frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);
    void plot(const QVector<QPointF>& pts, ContourRange& range, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);
    void plot(const QVector<QPointF>& pts, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);

//    void plot(const QVector<QPointF>& pts, const QSize& frameSize, const QColor& color, const QColor& tipColor, int width, Qt::PenStyle style);

    void emptyPlot(const QSize& frameSize);
    void emptyPlot();

    bool onResizeEvent(const QSize& size);

    static void copy_back(std::vector<fpoint_t>& pts, const std::vector<fpoint_t>& ent_pts);
    static void copy_back(QVector<QPointF>& pts, const std::vector<fpoint_t>& ent_pts);

    void addSymbolSquare(
        const fpoint_t* const botPoint,
        const QColor& botColor = Qt::GlobalColor::red,
        int size = INPUT_SQUARE_PX
    );

    bool isSwapXY() const;
    void setSwapXY(bool value);

    bool isInverseX() const;
    void setInverseX(bool value);

    bool isInverseY() const;
    void setInverseY(bool value);

    bool isShowXY() const;
    void setShowXY(bool value);

    static double calcPlotStep(double min, double max, size_t n);
};

#endif // PLOT_VIEW_H
