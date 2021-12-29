#ifndef WIRECUT_H
#define WIRECUT_H

#include <cstdint>
#include <vector>

#include <QString>
#include <QtXml>
#include <QFile>
#include "generator_mode_list.h"

enum class OFFSET_SIDE : uint8_t { LEFT, RIGHT };
OFFSET_SIDE operator~(const OFFSET_SIDE& side);

struct offset_t {
    uint8_t mode_id {0};
    double offset {0};

    offset_t(uint8_t mode_id = 0, double offset = 0) : mode_id(mode_id), offset(offset)  {}
};

struct cut_t {
    constexpr static const uint32_t MAX_TIMES = 7;
    constexpr static const double DEFAULT_SPEED = 3.6; // mm / min
    constexpr static const double DEFAULT_WIRE_DIAMETER = 0.33;

    uint8_t times;
    uint8_t cutline_mode_id;
    OFFSET_SIDE offset_side, aux_offset_side;
    bool offset_ena, aux_offset_ena, tab_multi_pass, tab_pause, pump_pause, speed_ena;
    uint16_t pump_delay, seg_pause; // sec
    size_t tab_seg;
    double aux_offset, tab, overlap, speed;

    std::vector<offset_t> offsets;
    offset_t tab_offset;
//    offset_t overlap_offset;
//    offset_t out_offset;

    cut_t() :
        times(1),
        cutline_mode_id(0),        
        offset_side(OFFSET_SIDE::RIGHT),
        aux_offset_side(OFFSET_SIDE::LEFT),
        offset_ena(false),
        aux_offset_ena(false),
        tab_multi_pass(true),
        tab_pause(false),
        pump_pause(true),
        speed_ena(true),
        pump_delay(60),
        seg_pause(0),
        tab_seg(0),
        aux_offset(0),
        tab(0),
        overlap(0),
        speed(DEFAULT_SPEED),
        tab_offset(offset_t())
//        overlap_offset(offset_t()),
//        out_offset(offset_t())
    {
        offsets.resize(MAX_TIMES);
        for (offset_t& o: offsets)
            o.offset = DEFAULT_WIRE_DIAMETER / 2.0;
    }

    uint8_t getTimes() const;
    std::vector<offset_t> getOffsets() const;
    std::vector<offset_t> getTabOffsets() const;
    offset_t getOverlapOffset() const;
    offset_t getOutOffset() const;
};

class WireCut : public cut_t {
    using cut_t::cut_t;

    QString m_error;

public:
    WireCut();
    WireCut(const cut_t& cut);
    const cut_t& get() const { return *this; }

    void write(QXmlStreamWriter& xml) const;
    bool saveXML(QFile& file, const GeneratorModeList& modes) const;

    bool parseOffset(QXmlStreamReader& xml, offset_t& offset, bool& valid);
    bool parseOffsets(QXmlStreamReader& xml, std::vector<offset_t>& offsets);
    bool parseCut(QXmlStreamReader& xml, cut_t& cut);
    bool openXML(QFile& file, GeneratorModeList& modes);

    bool hasError() const { return m_error.length() != 0; }
    const QString& errorString() const { return m_error; }
};

#endif // WIRECUT_H
