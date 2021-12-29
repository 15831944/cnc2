#include <QCoreApplication>
#include "my_lib.h"
#include <QDebug>

using namespace my_lib;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);    

//    double step = calcPlotStep(746, 1223, 10);
//    qDebug() << step;

//    step = calcPlotStep(0.012, 0.016, 10);
//    qDebug() << step;

//    std::vector<double> v = splitCircleByEqualSquare(100, 2);
//    print(v);

//    v = splitCircleByEqualSquare(100, 3);
//    print(v);

    std::vector<std::pair<double, double>> vv = splitCircle(100, 2, 300);
    print(vv);
    printf("\n");

//    vv = splitCircle(100, 4, 300);
//    print(vv);

    vv = splitCircle(100, 6, 300);
    print(vv);
    printf("\n");

    vv = splitCircle(100, 20, 300);
    print(vv);
    printf("\n");

    return a.exec();
}
