#ifndef GENERATORMODELIST_H
#define GENERATORMODELIST_H

#include <QtXml>
#include <QFile>
#include <deque>

struct GeneratorMode {
    float drum_velocity, pulse_width, pulse_ratio, voltage_level, current_level;

    GeneratorMode(float drumVelocity = 7, float pulseWidth = 36, float pulseRatio = 8, float voltageLevel = 0, float currentIndex = 7) :
        drum_velocity(drumVelocity), pulse_width(pulseWidth), pulse_ratio(pulseRatio), voltage_level(voltageLevel), current_level(currentIndex) {}
};

class GeneratorModeList {
    std::deque<GeneratorMode> m_list;
    QString m_error;
public:
    GeneratorModeList();
    GeneratorModeList(const std::deque<GeneratorMode>& modes);
    const std::deque<GeneratorMode>& get() const { return m_list; }

    void write(QXmlStreamWriter& xmlWriter) const;
    bool saveXML(QFile& file) const;

    bool parseMode(QXmlStreamReader& xml, GeneratorMode& mode, bool& valid);
    bool parseModes(QXmlStreamReader& xmlReader, std::deque<GeneratorMode>& modes);
    bool openXML(QFile& file);

    bool hasError() const { return m_error.length() != 0; }
    const QString& errorString() const { return m_error; }
};

#endif // GENERATORMODELIST_H
