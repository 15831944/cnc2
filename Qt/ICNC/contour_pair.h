#ifndef CONTOUR_PAIR_H
#define CONTOUR_PAIR_H

#include "dxf.h"

enum class CONTOUR_TYPE : uint8_t { CONTOUR_MAIN, CONTOUR_CUTLINE, CONTOUR_PASS, CONTOUR_TAB, CONTOUR_OVERLAP, CONTOUR_OUT };

class ContourPair {
    constexpr static const double M_PRECISION = PRECISION; // mm

    static GeneratorMode m_genModeDefault;
    static cut_t m_cutDefault;

    Dxf *m_bot, *m_top;

    CONTOUR_TYPE m_type;

    cut_t* m_cut;
    std::deque<GeneratorMode>* m_genModes;

    void lengthRatio(double len, double& bot_len, double& top_len) const;

public:
//    double speed() const { return m_cut ? m_cut->speed : m_cutDefault.speed; }

    class Reader {
        struct {
            uint8_t bot:1;
            uint8_t top:1;
        } valid;
        const ContourPair& m_pair;
        std::list<DxfEntity*>::const_iterator it_bot, it_top;

    public:
        Reader(const ContourPair& pair);
        const DxfEntity* bot();
        const DxfEntity* top();
        void next(const DxfEntity*& bot, const DxfEntity*& top);
    };

    Reader reader(const DxfEntity*& bot, const DxfEntity*& top) const;

    ContourPair(CONTOUR_TYPE type = CONTOUR_TYPE::CONTOUR_MAIN);
    ContourPair(const ContourPair& other);
    ContourPair(CONTOUR_TYPE type, const ContourPair& other);
    ContourPair(CONTOUR_TYPE type, const cut_t* const cut, const std::deque<GeneratorMode>* const modes);
    ContourPair(ContourPair&& other);
    ~ContourPair();

    ContourPair& operator=(const ContourPair& other);
    ContourPair& operator=(ContourPair&& other) noexcept;

    void setBot(const Dxf& ctr);
    Dxf* bot() const;

    void setTop(const Dxf& ctr);
    Dxf* top() const;

    void clear();

    void setType(CONTOUR_TYPE value) { m_type = value; }
    CONTOUR_TYPE type() const { return m_type; }

    void setCut(const cut_t& cut);
    void clearCut();
    const cut_t& cut() const;

    void setGeneratorModes(const std::deque<GeneratorMode>& modes);
    void setGeneratorModes(const ContourPair& contour);
    void clearModes();
    const GeneratorMode& generatorMode() const;
    void choiceMode(uint8_t mode_id);

    bool emptyBot() const;
    bool emptyTop() const;
    bool empty() const;
    bool isLoop() const;
    bool hasOut() const;

    bool isCutline() const;
    void setCutline();

    size_t countBot() const;
    size_t countTop() const;
    size_t count() const;

    std::string toStringShort() const;
    std::string toString() const;
    std::string typeToString() const;

    void reverse();

    void setFirst(size_t layer, size_t index);

    // Offset methods
    void split(double len, size_t& bot_idx, size_t& top_idx);
    void split_rev(double tab, size_t& bot_idx, size_t& top_idx);

    void remove_before(size_t bot_idx, size_t top_idx);
    void remove_to(size_t bot_idx, size_t top_idx);
    void remove_from(size_t bot_idx, size_t top_idx);
    void remove_after(size_t bot_idx, size_t top_idx);

    void offset(OFFSET_SIDE m_side, double offset);

    fpoint_valid_t firstBot() const;
    fpoint_valid_t firstTop() const;
    fpoint_valid_t lastBot() const;
    fpoint_valid_t lastTop() const;

    void changeFirstPoint(const fpoint_valid_t& m_bot, const fpoint_valid_t& m_top);
    void changeLastPoint(const fpoint_valid_t& m_bot, const fpoint_valid_t& m_top);

    void shift(const fpoint_t& pt);

    std::vector<fpoint_t> getPointsBot() const;
    std::vector<fpoint_t> getPointsTop() const;

    double lengthBot() const;
    double lengthBot(size_t index, const fpoint_t& pt) const;

    void rotate(const RotateMatrix& mx);
    void flipX(double x);
    void flipY(double y);
    void scale(double pct, const fpoint_t& base);

    ContourRange range() const;
};

#endif // CONTOUR_PAIR_H
