#include "wire_cut.h"

OFFSET_SIDE operator~(const OFFSET_SIDE &side) {
    return side == OFFSET_SIDE::LEFT ? OFFSET_SIDE::RIGHT : OFFSET_SIDE::LEFT;
}

WireCut::WireCut() {}

WireCut::WireCut(const cut_t& cut) {
    times = cut.times;
    cutline_mode_id = cut.cutline_mode_id;
    offset_side = cut.offset_side;
    tab_multi_pass = cut.tab_multi_pass;
    tab_pause = cut.tab_pause;
    pump_delay = cut.pump_delay;
    pump_pause = cut.pump_pause;
    tab = cut.tab;
    overlap = cut.overlap;

    offsets = cut.offsets;
    tab_offset = cut.tab_offset;
    m_error = QString();
}

void writeElement(QXmlStreamWriter& xml, const QString& name, const QString& type, const QString& value) {
    xml.writeStartElement(name);
        xml.writeAttribute("type", type);
        xml.writeCharacters(value);
    xml.writeEndElement();
}

void writeElement(QXmlStreamWriter& xml, const QString& name, float value) {
    writeElement(xml, name, "float", QString::number(static_cast<double>(value), 'g', 9));
}

void writeElement(QXmlStreamWriter& xml, const QString& name, unsigned value) {
    writeElement(xml, name, "unsigned", QString::number(value));
}

void writeElement(QXmlStreamWriter& xml, const QString& name, bool value) {
    writeElement(xml, name, "boolean", value ? "true" : "false");
}

void WireCut::write(QXmlStreamWriter& xml) const {
    xml.writeStartElement("cut");

        writeElement(xml, "offset_side", "OFFSET_SIDE", offset_side == OFFSET_SIDE::RIGHT ? "RIGHT" : "LEFT");
        writeElement(xml, "times", (unsigned)times);

        xml.writeStartElement("offsets");
            for (const offset_t& o: offsets) {
                xml.writeStartElement("offset");
                    writeElement(xml, "value", (float)o.offset);
                    writeElement(xml, "mode_id", (unsigned)o.mode_id);
                xml.writeEndElement();
            }
        xml.writeEndElement();

        writeElement(xml, "cutline_mode_id", (unsigned)cutline_mode_id);

        writeElement(xml, "tab_multi_pass", tab_multi_pass);
        writeElement(xml, "tab", (float)tab);

        xml.writeStartElement("tab_offset");
            writeElement(xml, "value", (float)tab_offset.offset);
            writeElement(xml, "mode_id", (unsigned)tab_offset.mode_id);
        xml.writeEndElement();

        writeElement(xml, "tab_pause", tab_pause);

        writeElement(xml, "overlap", (float)overlap);

        writeElement(xml, "pump_delay", (unsigned)pump_delay);
        writeElement(xml, "pump_pause", pump_pause);

        xml.writeEndElement();
}

bool WireCut::saveXML(QFile &file, const GeneratorModeList& modes) const {
    if (!file.isOpen())
        return false;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    write(xml);
    modes.write(xml);
    xml.writeEndDocument();

    return true;
}
