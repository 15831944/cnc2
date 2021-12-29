#ifndef CONTOURS_H
#define CONTOURS_H

#include <deque>
#include "contour_pair.h"
//#include "gcode.h"

class ContourList {
    std::deque<ContourPair> m_contours;
    const DxfEntity *m_selectedBot, *m_selectedTop;
    double m_thickness;
//    int contourSelected, entitySelected;

    int m_contour, m_segment;
    fpoint_t m_botPos, m_topPos;
    bool m_botValid, m_topValid;
    double m_botLength;

    std::string m_error;    

public:
    ContourList();
    ContourList(const ContourList& other);
    ContourList(const std::deque<ContourPair>& contours);
    ContourList(ContourList&& other);
    ContourList(std::deque<ContourPair>&& contours);

    ContourList& operator=(const ContourList& other);
    ContourList& operator=(ContourList&& other) noexcept;
    ~ContourList();

    void replace(std::deque<ContourPair>& contours);
//    void setCut(const cut_t& cut);
    void setCut(const cut_t& cut, const std::deque<GeneratorMode>& modes);
//    void setGeneratorModes(const std::deque<GeneratorMode>& modes);

    bool check();

    void push_front(const ContourPair& pair);
    void push_back(const ContourPair& pair);
    void insert(size_t index, const ContourPair& pair);
    void insert_after(size_t index, const ContourPair& pair);

    void new_insert(size_t index);
    void new_front();
    void new_back();

    void clear();
    bool remove(size_t index);

    ContourPair* at(size_t index);
    const ContourPair* at(size_t index) const;
    ContourPair* front();
    ContourPair* back();

    bool empty() const;
    size_t count() const;

    bool anyTop() const;
    bool verify();
    const std::string& lastError();

    ContourRange range() const;

    void select(size_t contour_num, size_t row, size_t column);
    void select(const std::pair<size_t, size_t>& ctr_ent);
    void clearSelected();
    const DxfEntity* selectedBot() const;
    const DxfEntity* selectedTop() const;
//    const DxfEntity* botSelected() const;
//    const DxfEntity* topSelected() const;
    bool isSelected() const;

    std::string toString() const;

    inline const std::deque<ContourPair>& contours() const { return m_contours; }

    static void shift(std::deque<ContourPair>& contours, const fpoint_t& pt);
    void shift(const fpoint_t& pt);
    void shift();
    fpoint_valid_t firstBot() const;
    fpoint_valid_t firstTop() const;

    void setBotPos(const fpoint_t& pt);
    void setTopPos(const fpoint_t& pt);
    void clearPos();
    bool isBotPos() const;
    bool isTopPos() const;
    const fpoint_t* botPos() const;
    const fpoint_t* topPos() const;

    void setCurrent(int contour_num, int segment_num);
    int currentContourNumber() const { return m_contour; }
    int currentSegmentNumber() const { return m_segment; }

    double botLengthFull();
    double botLength() const;
    double pct();

    void rotate(double phi);
    void flipX();
    void flipY();
};

#endif // CONTOURS_H
