#ifndef DXF_ENTITY_H
#define DXF_ENTITY_H

#include <cstdint>
#include <string>
#include <deque>
#include <vector>
#include <QColor>
#include "fpoint_t.h"
#include "main.h"
#include "cnc_types.h"
#include "contour_range.h"

enum class ENTITY_TYPE : int8_t { NONE = -1, LINE, ARC, CIRCLE, POINT, INSERT };
enum class CUT_TYPE : uint8_t { CUT_BODY, INCUT, OUTCUT };
enum class DIRECTION : uint8_t { Q1, Q2, Q3, Q4 };

bool almost_same(DIRECTION dir1, DIRECTION dir2);

class IContourObject {
protected:
    IContourObject();
public:
    virtual ~IContourObject() = 0;
    virtual fpoint_t point_0() const = 0;
    virtual fpoint_t point_1() const = 0;
    virtual fpoint_t center() const = 0;
    virtual double radius() const { return 0; }
    virtual double tangent_0() const { return 0; }
    virtual double tangent_1() const { return 0; }
    virtual double length() const { return 0; }
    virtual double length(const fpoint_t&) const { return 0; }
    virtual double dx() const { return 0; }
    virtual double dy() const { return 0; }
    virtual std::vector<fpoint_t> getPoints() const { return std::vector<fpoint_t>(); }
    virtual bool isPoint() const = 0;
    virtual bool valid() const { return false; }
    virtual void clear() {}
    virtual void change_0(const fpoint_t&) {}
    virtual void change_1(const fpoint_t&) {}

    virtual ContourRange range() const = 0;
//    virtual DIRECTIONS direction_0() { return DIRECTIONS::Q1; }
//    virtual DIRECTIONS direction_1() { return DIRECTIONS::Q1; }
};

class DxfEntity : public IContourObject {
protected:
    constexpr static const double M_PRECISION = PRECISION; // mm
    ENTITY_TYPE m_type;

    struct {
        bool valid;
        double value; // mm/min
    } m_speed {0, false};

    struct {
        uint8_t value;
        bool valid;
    } m_drum_vel {0, false};

    DxfEntity(ENTITY_TYPE type = ENTITY_TYPE::NONE);
    DxfEntity(const DxfEntity& other, ENTITY_TYPE type);

public:
//    CUT_TYPES cut_type;
//    bool highlight;

    virtual ~DxfEntity() override = 0;

    virtual DxfEntity* clone() const = 0;

    virtual bool operator==(const DxfEntity& /*other*/) const { return false; }
    virtual bool operator!=(const DxfEntity& /*other*/) const { return false; }
    virtual bool same(const DxfEntity& /*other*/) const { return false; }

    virtual bool check() { return false; }

    virtual DxfEntity* trim_front(double head_length, bool rem = false);
    virtual DxfEntity* trim_front_rev(double tail_length, bool rem = false);
    virtual DxfEntity* trim_back(double tail_length, bool rem = false);
    virtual DxfEntity* trim_back_rev(double head_length, bool rem = false);
    virtual void offset(OFFSET_SIDE /*side*/, double /*offset*/, const DxfEntity* /*prev*/, const DxfEntity* /*next*/) {}
    virtual void offset(OFFSET_SIDE /*side*/, double /*offset*/) {}

    virtual void rotate(const RotateMatrix& /*mx*/) {}
    virtual void flipX(double /*x*/) {}
    virtual void flipY(double /*y*/) {}
    virtual void scale(double /*k*/) {}

    ENTITY_TYPE type() const;
    std::string typeString() const;

    virtual std::string blockName() const;

    virtual void shift(const fpoint_t& value) = 0;
    virtual void reverse() = 0;    

    virtual std::string toString() const = 0;

    virtual double length() const override { return 0; }
    virtual double length(const fpoint_t&) const override { return 0; }

    virtual void setAdditional(bool /*value = true*/) {}
    virtual bool additional() { return false; }

    virtual bool CCW() const { return 0; }

    void setSpeed(double value) {
        m_speed.value = value;
        m_speed.valid = true;
    }
    double speed() const { return m_speed.valid ? m_speed.value : 0; }
    void invalidateSpeed() { m_speed.valid = false; }
    bool hasSpeed() const { return m_speed.valid; }

    void setDrumVel(double value) {
        m_drum_vel.value = value;
        m_drum_vel.valid = true;
    }
    double drumVel() const { return m_drum_vel.valid ? m_drum_vel.value : 0; }
    void invalidateDrumVel() { m_drum_vel.valid = false; }
    bool hasDrumVel() const { return m_drum_vel.valid; }

    static double lengthSquare(const fpoint_t& A, const fpoint_t& B);
    static double length(const fpoint_t& A, const fpoint_t& B);
    static DIRECTION dir(double dx, double dy);
    static DIRECTION dir(const fpoint_t& A, const fpoint_t& B);
    static bool almost_same_dir(const fpoint_t& base, const fpoint_t& A, const fpoint_t& B);
};

#endif // DXF_ENTITY_H
