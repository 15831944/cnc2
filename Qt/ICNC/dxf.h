#ifndef DXF_H
#define DXF_H

#include "dxf_entities.h"
#include <algorithm>
#include "main.h"
#include <QVector>
#include <QPointF>
#include "frect_t.h"
#include "contour_range.h"
#include "my_types.h"
#include "my_lib.h"
#include <QList>

struct DxfCodeValue {
    int16_t code;
    std::string value;

    bool equal(int16_t code, const std::string& value) const;
    double getDouble() const;
};

class Dxf {
    constexpr static const double M_PRECISION = PRECISION; // mm

    enum class STATES {SKIP, SECTION, BLOCKS, BLOCK, BLOCK_LINE, BLOCK_ARC, BLOCK_CIRCLE, BLOCK_POINT, BLOCK_ENTITY, ENTITIES, LINE, ARC, CIRCLE, POINT, INSERT, ENTITY};
    static STATES next_entity(Dxf::STATES state, const DxfCodeValue& pair);
    STATES next_block_entity(Dxf::STATES state, const DxfCodeValue& pair, DxfBlock*& block);

    std::string m_fileDir, m_fileName;
    FILE* m_fp;
    int m_ch_reg;

    std::deque<DxfBlock*> m_blocks;
    std::list<DxfEntity*> m_entities;

    QColor m_color;

    std::string m_last_error;

    size_t m_outEntityNum;
    bool m_out_valid;

//    double x_min, x_max, y_min, y_max;

    std::string readString();
    DxfCodeValue getCodeValue();

    void clearEntities();
    void clearBlocks();

    void addInsert(const DxfInsert& insert);
    const DxfBlock* getBlockByName(const std::string& blockName) const;

    std::list<DxfEntity*>::iterator replaceBlock(std::list<DxfEntity*>::iterator it);
    void replaceBlocks();

    static bool searchNext(
            const fpoint_t& pt1,
            std::list<DxfEntity*>::iterator& it,
            const std::list<DxfEntity*>::iterator& begin,
            const std::list<DxfEntity*>::iterator& end
    );

    static bool searchPrev(
            const fpoint_t& pt0,
            std::list<DxfEntity*>::iterator& it,
            const std::list<DxfEntity*>::iterator& begin,
            const std::list<DxfEntity*>::iterator& end
    );

    static size_t size(const std::list<DxfEntity*> entities);

public:
    Dxf();
    Dxf(const Dxf& other);
    Dxf(Dxf&& other);
    ~Dxf();

    Dxf& operator=(const Dxf& other);
    Dxf& operator=(Dxf&& other) noexcept;

    void init(const std::string& fileDir, const std::string& fileName);
    inline const std::string& fileName() const { return m_fileName; }

    bool open();
    inline bool isOpen() { return m_fp != nullptr; }
    void close();
    bool parse();
    void clear();

    void cutUnconnected(std::list<DxfEntity*>& free);
    void cutTails(std::list<DxfEntity*>& tails);
    void movetoHeadTail(std::list<DxfEntity*>& tails);

    bool sort(Dxf& free, Dxf& unused);

    void reverse();
    void setFirst(size_t index);

    void push_back(DxfEntity* const entity);
    void push_back(const DxfEntity& entity);

    void push_front(DxfEntity* const entity);
    void push_front(const DxfEntity& entity);

    fpoint_valid_t first_point() const;
    fpoint_valid_t last_point() const;
    const DxfEntity* at(size_t index) const;

    inline const std::list<DxfEntity*>& entities() const { return m_entities; }
    bool isLoop() const;
    bool hasOut() const;
    size_t outEntityNum() const;
    std::string lastError();
//    GCode moveToGCode();

    static ContourRange contourRange(const std::list<DxfEntity*>& contour);
    ContourRange contourRange() const;

    void printDebug() const;
    std::string toString() const;

    const QColor& color() const { return m_color; }
    size_t count() const;
    bool empty() const { return m_entities.empty(); }

    Dxf cut_front();
    Dxf cut_back();    

    DxfEntity* front() const { return m_entities.empty() ? nullptr : m_entities.front(); }
    DxfEntity* back() const { return m_entities.empty() ? nullptr : m_entities.back(); }

    fpoint_valid_t first() const { return m_entities.empty() ? fpoint_valid_t(false) : m_entities.front()->point_0(); }
    fpoint_valid_t last() const { return m_entities.empty() ? fpoint_valid_t(false) : m_entities.back()->point_1(); }

    // Offset methods
    double length() const;
    double length(size_t index, const fpoint_t& pt) const;

    Dxf* copy_front(double length) const;
    Dxf* copy_front_rev(double tab) const;
    Dxf* copy_back(double length) const;

    bool offset(OFFSET_SIDE offset_side, double offset);
    bool intersectLine(DxfEntity*& A, DxfEntity*& B);
    bool intersect(bool loop, OFFSET_SIDE side, double offset);
    void connectLoop();
    void alignDxfPoints();
    static std::list<DxfEntity*>::iterator nextLineOrArc(
            const std::list<DxfEntity*>::iterator& init,
            const std::list<DxfEntity*>::iterator& begin,
            const std::list<DxfEntity*>::iterator& end,
            bool& stop_req,
            bool loop = false
        );
    static std::list<DxfEntity*>::iterator prevLineOrArc(
            const std::list<DxfEntity*>::iterator& init,
            const std::list<DxfEntity*>::iterator& begin,
            const std::list<DxfEntity*>::iterator& end,
            bool& stop_req,
            bool loop = false
        );

    // return first entity index after split in forward order
    size_t split(double length);

    // return first entity index after split in forward order
    size_t split_rev(double tab);

    void remove_before(size_t index);
    void remove_to(size_t index);
    void remove_from(size_t index);
    void remove_after(size_t index);

    void changeFirstPoint(const fpoint_valid_t& pt);
    void changeLastPoint(const fpoint_valid_t& pt);

    void shift(const fpoint_t& pt);

    std::vector<fpoint_t> getPoints() const;

    size_t joinedCount() const;
    bool whole() const;

    frect_t getRectangle() const;

    void rotate(const RotateMatrix& mx);
    void flipX(double x);
    void flipY(double y);
    void scale(double k);

private:
    void addRectSpeedProfile(double width, double speed, bool fwd, fpoint_t& A, fpoint_t& B);
    double addCircleSpeedProfile(const slicing_t& par, bool fwd, fpoint_t& A, fpoint_t& B);
    double addSlice(const slicing_t& par, bool fwd, bool last, fpoint_t& A, fpoint_t& B);
    double addComb(const comb_t& par, bool fwd, bool last, fpoint_t& A, fpoint_t& B);

public:
    void generate(const slicing_t& par);
    void generate(const comb_t& par);
};

#endif // DXF_H
