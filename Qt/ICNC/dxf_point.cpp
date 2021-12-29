#include "dxf_point.h"

using namespace std;

DxfPoint::DxfPoint(bool additional) : DxfEntity(ENTITY_TYPE::POINT), flags({0,0,0,additional}), pt(fpoint_t()) {}
DxfPoint::DxfPoint(const fpoint_t &point, bool additional) :
    DxfEntity(ENTITY_TYPE::POINT),
    flags({0,0,1,additional}),
    pt(point)
{
}
DxfPoint::DxfPoint(double x, double y, bool additional) :
    DxfEntity(ENTITY_TYPE::POINT),
    flags({0,0,1,additional}),
    pt(fpoint_t(x, y))
{
}
DxfPoint::DxfPoint(const DxfPoint &other) : DxfEntity(ENTITY_TYPE::POINT), flags(other.flags), pt(other.pt) {}
DxfPoint::~DxfPoint() {}

DxfEntity *DxfPoint::clone() const { return new DxfPoint(*this); }

void DxfPoint::setX(double value) {
    flags.valid = 0;
    flags.X = 1;
    pt.x = value;
}

void DxfPoint::setY(double value) {
    flags.valid = 0;
    flags.Y = 1;
    pt.y = value;
}

void DxfPoint::set(const fpoint_t& point, bool additional) {
    flags = {0,0,1,additional};
    pt = point;
}

bool DxfPoint::check() {
    if (!flags.valid && flags.X && flags.Y)
        flags = {0,0,1,0};

    return flags.valid;
}

void DxfPoint::clear() {
    flags = {0,0,0,0};
    pt = fpoint_t();
}

void DxfPoint::flipX(double x) {
    pt.flipX(x);
}

void DxfPoint::flipY(double y) {
    pt.flipY(y);
}

void DxfPoint::scale(double k) {
    pt.scale(k);
}

string DxfPoint::toString() const {
    return flags.valid ? "Point: " + pt.toString() : "Not valid";
}

ContourRange DxfPoint::range() const { return ContourRange(pt.x, pt.x, pt.y, pt.y); }
