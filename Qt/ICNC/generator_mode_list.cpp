#include "generator_mode_list.h"

GeneratorModeList::GeneratorModeList() {}

GeneratorModeList::GeneratorModeList(const std::deque<GeneratorMode>& modes) {
    m_list = modes;
    m_error = QString();
}

void GeneratorModeList::write(QXmlStreamWriter& xmlWriter) const {
    xmlWriter.writeStartElement("modes");

    for (const GeneratorMode& m : m_list) {
        xmlWriter.writeStartElement("mode");
            xmlWriter.writeStartElement("drum_velocity");
            xmlWriter.writeAttribute("type", "float");
            xmlWriter.writeCharacters(QString::number(static_cast<double>(m.drum_velocity), 'g', 9));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("pulse_width");
            xmlWriter.writeAttribute("type", "float");
            xmlWriter.writeCharacters(QString::number(static_cast<double>(m.pulse_width), 'g', 9));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("pulse_ratio");
            xmlWriter.writeAttribute("type", "float");
            xmlWriter.writeCharacters(QString::number(static_cast<double>(m.pulse_ratio), 'g', 9));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("voltage_level");
            xmlWriter.writeAttribute("type", "float");
            xmlWriter.writeCharacters(QString::number(static_cast<double>(m.voltage_level), 'g', 9));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("current_level");
            xmlWriter.writeAttribute("type", "float");
            xmlWriter.writeCharacters(QString::number(static_cast<double>(m.current_level), 'g', 9));
            xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();
}

bool GeneratorModeList::saveXML(QFile& file) const {
    if (!file.isOpen())
        return false;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    write(xml);
    xml.writeEndDocument();

    return true;
}

float readFloat(QXmlStreamReader& xml, bool* ok = nullptr) {
    QString s = xml.readElementText();
    return s.toFloat(ok);
}

bool GeneratorModeList::parseMode(QXmlStreamReader& xml, GeneratorMode& mode, bool& valid) {
    bool ok;
    float value;
    uint flags = 0;

    if (xml.atEnd() || xml.hasError()) {
        valid = false;
        return false;
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "mode")
        xml.readNext();
    else {
        valid = false;
        return false;
    }

    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "mode") {
            xml.readNext();
            valid = flags == 0x1F;
            return true;
        }

        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "drum_velocity") {
                value = readFloat(xml, &ok);
                if (ok) {
                    mode.drum_velocity = value;
                    flags |= 1;
                }
            }
            else if (xml.name() == "pulse_width") {
                value = readFloat(xml, &ok);
                if (ok) {
                    mode.pulse_width = value;
                    flags |= 2;
                }
            }
            else if (xml.name() == "pulse_ratio") {
                value = readFloat(xml, &ok);
                if (ok) {
                    mode.pulse_ratio = value;
                    flags |= 4;
                }
            }
            else if (xml.name() == "voltage_level") {
                value = readFloat(xml, &ok);
                if (ok) {
                    mode.voltage_level = value;
                    flags |= 8;
                }
            }
            else if (xml.name() == "current_level") {
                value = readFloat(xml, &ok);
                if (ok) {
                    mode.current_level = value;
                    flags |= 0x10;
                }
            }
        }

        xml.readNext();
    }

    valid = false;
    return false;
}

bool GeneratorModeList::parseModes(QXmlStreamReader& xml, std::deque<GeneratorMode>& modes) {
    GeneratorMode mode;
    bool valid;

    modes.clear();

    if (xml.atEnd() || xml.hasError())
        return false;
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "modes")
        xml.readNext();
    else
        return false;

    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "modes") {
            xml.readNext();
            return true;
        }

        bool ok = parseMode(xml, mode, valid);

        if (valid)
            modes.push_back(mode);

        if (!ok)
            xml.readNext();
    }

    return false;
}

bool GeneratorModeList::openXML(QFile& file) {
    QXmlStreamReader xml;
    xml.setDevice(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartDocument)
            break;
    }

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (parseModes(xml, m_list))
                break;
            else
                xml.readNext();
        }
    }

    if (xml.hasError()) {
        m_error = xml.errorString();
        return false;
    }

//    xml.clear();

    return !m_list.empty();
}
