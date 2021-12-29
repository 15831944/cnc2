#include "my_lib.h"
#include <cmath>
#include <cstdio>

double my_lib::calcPlotStep(double min, double max, size_t n) {
    double w = max - min;
    double step = w / n;

    double e = log10(step);
    e = trunc(e);
    double exp = pow(10, e);

    if (step / exp < 1)
        exp = e < 0 ? exp / 10 : exp * 10;

    step /= exp; // 1..9
    step = round(step);

    if (step <= 2)
        step = 2;
    else if (step <= 5)
        step = 5;
    else
        step = 10;

    return step * exp;
}

// alpha = 0..PI
double  my_lib::circleSegmentSquareAlpha(double R, double alpha) {
    return (alpha >= 0 && alpha <= M_PI) ? R*R * (alpha - sin(alpha)) / 2 : 0;
}

// d = 0..R
double  my_lib::circleSegmentSquareNorm(double R, double d) {
    if (d >= 0 && d <= R) {
        double alpha = 2.0 * acos(d / R);
        return circleSegmentSquareAlpha(R, alpha);
    }

    return 0;
}

using namespace  std;

vector<double>  my_lib::splitCircleByEqualSquare(double R, unsigned n) {
    const int m = 1000;
    size_t N;
    double S, Sn, dd;

    vector<double> res(n+1);
    res.clear();

    if (n == 0)
        return res;
    else if (n == 1) {
        res.push_back(R);
        res.push_back(0);
        return res;
    }

    S = M_PI * R * R / 2;
    Sn = S / n;
    N = n * m;
    dd = R / N;

    res.push_back(R);
    double sum = 0;
    for (unsigned i = N - 1; i > 0; i--) {
        double d = dd * i;
        double Sseg = circleSegmentSquareNorm(R, d);

        if (Sseg - sum >= Sn) {
            res.push_back(d);

            if (res.size() == n)
                break;

            sum = Sseg;
        }
    }

    res.resize(n+1);
    res[n] = 0;

    return res;
}

void my_lib::print(const vector<double>& v) {
    for (unsigned i = 0; i < v.size(); i++) {
        printf("%d: %f\n", i, v[i]);
    }
}

vector<pair<double, double>>  my_lib::splitCircle(double R, unsigned n, double Vavg) {
    unsigned half = (n & 1) ? n / 2 + 1 : n / 2;

    const vector<double> v = splitCircleByEqualSquare(R, half);
    vector<pair<double, double>> res(2 * half);
    res.clear();

    if (v.size()) {
        for (unsigned i = 0; i < (v.size() - 1); i++)
            res.push_back( pair<double, double>(v[i] - v[i + 1], 0) );

        for (unsigned i = (v.size() - 1); i > 0; i--)
            res.push_back( pair<double, double>(v[i - 1] - v[i], 0) );

        double t = 2 * R / Vavg;
        double dt = t / (2 * half);

        for (unsigned i = 0; i < res.size(); i++)
            res[i].second = res[i].first / dt;
    }

    return res;
}

void my_lib::print(const vector<pair<double, double>>& v) {
    for (unsigned i = 0; i < v.size(); i++) {
        printf("%d: L = %f, Vel = %f\n", i, v[i].first, v[i].second);
    }
}
