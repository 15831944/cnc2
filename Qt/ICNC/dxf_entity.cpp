#include "dxf_entity.h"
#include <cmath>

using namespace std;

IContourObject::IContourObject() {}
IContourObject::~IContourObject() {}

DxfEntity::DxfEntity(ENTITY_TYPE type) : m_type(type) {}

DxfEntity::DxfEntity(const DxfEntity& other, ENTITY_TYPE type) : m_type(type), m_speed(other.m_speed), m_drum_vel(other.m_drum_vel) {}

DxfEntity::~DxfEntity() {}

DxfEntity *DxfEntity::trim_front(double, bool) { return nullptr; }
DxfEntity *DxfEntity::trim_front_rev(double, bool) { return nullptr; }
DxfEntity *DxfEntity::trim_back(double, bool) { return nullptr; }
DxfEntity *DxfEntity::trim_back_rev(double, bool) { return nullptr; }

ENTITY_TYPE DxfEntity::type() const { return m_type; }

string DxfEntity::typeString() const {
    switch (m_type) {
    case ENTITY_TYPE::NONE: return "NONE";
    case ENTITY_TYPE::LINE: return "LINE";
    case ENTITY_TYPE::ARC: return "ARC";
    case ENTITY_TYPE::CIRCLE: return "CIRCLE";
    case ENTITY_TYPE::POINT: return "POINT";
    case ENTITY_TYPE::INSERT: return "INSERT";
    }

    return string();
}

string DxfEntity::blockName() const { return string(); }

double DxfEntity::lengthSquare(const fpoint_t& A, const fpoint_t& B) {
    double dx = B.x - A.x;
    double dy = B.y - A.y;
    return dx * dx + dy * dy;
}

double DxfEntity::length(const fpoint_t& A, const fpoint_t& B) {
    return sqrt(lengthSquare(A, B));
}

DIRECTION DxfEntity::dir(double dx, double dy) {
    if (dx >= 0)
        return dy >= 0 ? DIRECTION::Q1 : DIRECTION::Q4;
    else
        return dy >= 0 ? DIRECTION::Q2 : DIRECTION::Q3;
}

DIRECTION DxfEntity::dir(const fpoint_t& A, const fpoint_t& B) {
    return dir(B.x - A.x, B.y - A.y);
}

bool almost_same(DIRECTION dir1, DIRECTION dir2) {
    return (dir1 == DIRECTION::Q1 && dir2 != DIRECTION::Q3) ||
           (dir1 == DIRECTION::Q2 && dir2 != DIRECTION::Q4) ||
           (dir1 == DIRECTION::Q3 && dir2 != DIRECTION::Q1) ||
           (dir1 == DIRECTION::Q4 && dir2 != DIRECTION::Q2);
}

bool DxfEntity::almost_same_dir(const fpoint_t& base, const fpoint_t& A, const fpoint_t& B) {
    return almost_same(dir(base, A), dir(base, B));
}
