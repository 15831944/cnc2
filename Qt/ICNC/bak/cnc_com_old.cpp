//#include "cnc_com.h"

//#include <iostream>
//#include <memory>
//#include <iostream>
//#include <string>
//#include <cstdio>
//#include <stdexcept>

//#include "aux_func.h"

//#include <QDebug>
//#include <QThread>
//#include <QTime>
//#include <QCoreApplication>
////#include <regex>
//#include "main.h"

//using namespace std;
//using namespace aux_func;

//CncCom::CncCom(ReportWriter txt): txt(txt) {
//    timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, &CncCom::onTimeOut);
//}

//CncCom::~CncCom() {
//    if (m_port) {
//        delete m_port;
//        m_port = nullptr;
//    }
//}

//// Open port or close and reopen
//bool CncCom::open() {
//    QString portName;

//    // Print all serial porta and open the last suitable
//    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
//    for (const QSerialPortInfo& info: list) {
//        if (!info.isNull()) {
//            qDebug("name: %s PRODUCT_ID: %x VENDOR_ID: %x", info.portName().toStdString().c_str(), info.productIdentifier(), info.vendorIdentifier());

//            if (info.productIdentifier() == ST_PRODUCT_ID && info.vendorIdentifier() == ST_VENDOR_ID) {
//                portName = info.portName();
//            }
//        }
//    }

//    if (m_port) {
//        if (m_port->isOpen())
//            m_port->close();

//        delete m_port;
//        m_port = nullptr;
//    }

//    if (portName.size() != 0) {
//        m_port = new QSerialPort();
//        m_port->setPortName(portName);

//        // uart port parameters need for correct work, but don't use
//        m_port->setBaudRate(QSerialPort::Baud115200);
//        m_port->setDataBits(QSerialPort::Data8);
//        m_port->setParity(QSerialPort::NoParity);
//        m_port->setStopBits(QSerialPort::OneStop);
//        m_port->setFlowControl(QSerialPort::NoFlowControl);
//        m_port->setReadBufferSize(16 * 1024);

//        QString s;
//        if (!m_port->open(QIODevice::ReadWrite)) {
//            delete m_port;
//            m_port = nullptr;
//            s = QString::asprintf("Port %s is not opened", portName.toStdString().c_str());
//        }
//        else {
//            s = QString::asprintf("Port %s is opened", portName.toStdString().c_str());

//            connect(m_port, &QSerialPort::readyRead, this, &CncCom::onReadyData);
//            connect(m_port, &QSerialPort::bytesWritten, this, &CncCom::onBytesWritten);
//        }

//        txt.write(s);
//        return m_port != nullptr;
//    }
//    else {
//        txt.write("STM USB CDC device was not found");
//        return false;
//    }
//}

//// Close port
//bool CncCom::close() {
//    if (m_port && m_port->isOpen()) {
//        m_port->close();
//        delete m_port;
//        m_port = nullptr;
//        QString s = "COM port is closed";
//        txt.write(s);
//        return true;
//    }

//    if (m_port) delete m_port;
//    m_port = nullptr;
//    return false;
//}

//void CncCom::writeBytesAsync(uint32_t addr, const std::vector<uint8_t> &bytes, size_t begin, size_t length) {
//    if (length != 0 && m_port != nullptr && m_port->isOpen() && !m_busy) {
//        m_busy = true;
//        m_port->clear();
//        txd.createWritePacket(addr, bytes, begin, length);
//        m_port->write(reinterpret_cast<const char*>(txd.rawData()), qint64(txd.rawSize()));
//        m_port->flush();
//    } else
//        onTimeOut();
//}

//void CncCom::readBytesAsync(uint32_t addr, size_t length, ComPacket::Command command) {

//}

////#define RX_PRINT
///* Read data from port until pause "timeout" ms
// * return - number of received bytes or -1 timeout
// */
//int CncCom::readPort(ComPacket& rxd, int timeout) {
//    if (m_port) {
//        do {
//            bool OK = m_port->waitForReadyRead(timeout);

//            if (OK) {
//                QByteArray qres = m_port->readAll();

//                for (QByteArray::iterator it = qres.begin(); it != qres.begin(); ++it) {
//                    rxd.append(*it);

//                    if (rxd.complete())
//                        break;
//                }
//            }
//            else {
//                rxd.finalize();
//                return -2;
//            }

//        } while (!rxd.complete());

//#ifdef RX_PRINT
//        static int n;

//        if (rxd.empty())
//            n = 0;

//        if (!rxd.empty()) {
//            for (size_t i = 0; i < rxd.count(); i++) {
//                if (n != 15)
//                    qDebug("%02X ", int(rxd.rawData(i)));
//                else
//                    qDebug("%02X\n", int(rxd.rawData(i)));
//            }
//        }
//        else {
//            if (n != 15)
//                qDebug("Empty packet ");
//            else
//                qDebug("Empty packet\n");
//        }
//        n++;
//#endif
//        return rxd.length();
//    }
//    else {
//        rxd.finalize();
//        return -1;
//    }
//}

///* Send bytes
// * addr - destination address
// * bytes - data
// * begin - start possition in data array
// * length - number of bytes, 256 max
// * Throw std::runtime_error if error
// */
//void CncCom::writeBytes(uint32_t addr, const vector<uint8_t>& bytes, size_t begin, size_t length) {
//    if (length != 0) {
//        if ((addr & 0xF0000000) != 0)
//            throw runtime_error( string_format("Address error: 0x%08x", int(addr)) );

//        if (!m_port || !m_port->isOpen())
//            throw runtime_error("COM port isn't opened");

//        txd.createWritePacket(addr, bytes, begin, length);

//        m_port->write(reinterpret_cast<const char*>(txd.rawData()), qint64(txd.rawSize()));
//        m_port->flush();

//#ifdef PRINT_CNC_COM_DEBUG
//        qDebug("Write to address 0x%08x:\n", addr);
//        aux_func::print_array(txd.rawData(), txd.rawSize());
//#endif

//        while (true) {
//            int res = readPort(rxd, TIMEOUT);

//            if (res <= 0)
//                throw runtime_error( string("Rx timeout. Received ") + to_string(rxd.rawSize()) + " bytes" );

//            if (rxd.complete()) {
//                if (rxd.command() == ComPacket::Command::CMD_ERROR)
//                    throw runtime_error("CNC sent an error code");

//                if (rxd.error()) {
//                    switch (rxd.error_code()) {
//                    case 1: throw runtime_error("Write acknowledge. Unknown command");
//                    case 2: throw runtime_error("Write acknowledge. CRC Error");
//                    case 3: throw runtime_error("Write acknowledge. Timeout Error");
//                    default: throw runtime_error("Write acknowledge. Unknown Error");
//                    }
//                }

//                if (rxd.command() != ComPacket::Command::CMD_WRITE)
//                    throw runtime_error(string_format("Write acknowledge. Command error: 0x%x", int(rxd.command())));

//                if (rxd.addr() != addr)
//                    throw runtime_error(string_format("Write acknowledge. Address error: 0x%x", int(rxd.addr())));

//                if (rxd.length() != length)
//                    throw runtime_error(string_format("Write acknowledge. Length error: %d", int(rxd.length())));

//                break;
//            }
//        }
//    }
//}

//void CncCom::writeBytes(uint32_t addr, const uint8_t* data, size_t size, size_t begin, size_t length) {
//    if (begin < size) {
//        data += begin;

//        if (begin + length > size)
//            length = static_cast<uint8_t>(size - begin);

//        if (length != 0) {
//            vector<uint8_t> bytes(length);
//            memcpy(bytes.data(), data, length);
//            writeBytes(addr, bytes, 0, length);
//        }
//    }
//}

//void CncCom::write(uint32_t addr, const void* data, size_t size) {
//    const size_t max = ComPacket::MAX;
//    size_t pos = 0;

//    while (size > 0) {
//        size_t len = size > max ? max : static_cast<size_t>(size);

//        this->writeBytes(addr, reinterpret_cast<const uint8_t*>(data), size, pos, len);

//        addr += len;
//        pos += len;
//        size -= len;
//    }
//}

//void CncCom::write(uint32_t addr, const vector<uint8_t>& bytes) {
//    write(addr, bytes.data(), bytes.size());
//}

//// no reverse!
//void CncCom::write16(uint32_t addr, uint16_t data) {
//    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
//    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
//    this->write(addr, v);
//}

//void CncCom::write32(uint32_t addr, uint32_t data) {
//    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
//    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
//    this->write(addr, v);
//}

//void CncCom::write64(uint32_t addr, uint64_t data) {
//    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
//    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
//    this->write(addr, v);
//}

//void CncCom::write48(uint32_t addr, uint64_t data) {
//    vector<uint8_t> v = vector<uint8_t>(sizeof(uint16_t) * 3);
//    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
//    this->write(addr, v);
//}

//vector<uint8_t> CncCom::readBytes(uint32_t addr, size_t length, ComPacket::Command cmd) {
//    vector<uint8_t> rddata(length);

//    if (length != 0 && (cmd == ComPacket::Command::CMD_READ || cmd == ComPacket::Command::CMD_READ_FIFO)) {
//        if ((addr & 0xF0000000) != 0)
//            throw runtime_error( string_format("Address error: 0x%08x", int(addr)) );

//        if (!m_port || !m_port->isOpen())
//            throw runtime_error("COM port isn't opened");

//        txd.createReadPacket(cmd, addr, length);

//        m_port->write(reinterpret_cast<const char*>(txd.rawData()), qint64(txd.rawSize()));
//        m_port->flush();

//#ifdef PRINT_CNC_COM_DEBUG
//        qDebug("Write to address 0x%08x:\n", addr);
//        aux_func::print_array(txd.rawData(), txd.rawSize());
//#endif

//        while (true) {
//            int res = readPort(rxd, TIMEOUT);

//            if (res <= 0)
//                throw runtime_error( string("Rx timeout. Received ") + to_string(rxd.rawSize()) + " bytes" );

//            if (rxd.complete()) {
//                if (rxd.command() == ComPacket::Command::CMD_ERROR)
//                    throw runtime_error("CNC sent an error code");

//                if (rxd.error()) {
//                    switch (rxd.error_code()) {
//                    case 1: throw runtime_error("Write acknowledge. Unknown command");
//                    case 2: throw runtime_error("Write acknowledge. CRC Error");
//                    case 3: throw runtime_error("Write acknowledge. Timeout Error");
//                    default: throw runtime_error("Write acknowledge. Unknown Error");
//                    }
//                }

//                if (rxd.command() != cmd)
//                    throw runtime_error(string_format("Write acknowledge. Command Error: 0x%x", int(rxd.command())));

//                if (rxd.addr() != addr)
//                    throw runtime_error(string_format("Write acknowledge. Address Error: 0x%x", int(rxd.addr())));

//                if (rxd.length() != length)
//                    throw runtime_error(string_format("Write acknowledge. Length Error: %d", int(rxd.length())));

//                break;
//            }
//        }
//    }

//    memcpy(rddata.data(), rxd.data(), rddata.size());
//    return rddata;
//}

//std::vector<uint8_t> CncCom::readFifo(uint32_t addr, size_t length) {
//    return readBytes(addr, length, ComPacket::Command::CMD_READ_FIFO);
//}

//void CncCom::onReadyData() {

//}

//void CncCom::onTimeOut() {
//    rxd.clear();
//    emit response(rxd);
//}

//vector<uint8_t> CncCom::read(uint32_t addr, size_t len) {
//    vector<uint8_t> v;
//    const size_t max = ComPacket::MAX;
//    size_t rem = len;

//    while (rem > 0) {
//        size_t size = rem > max ? max : static_cast<size_t>(rem);

//        vector<uint8_t> packet = readBytes(addr, size);
//        push_back_range(v, packet, 0, packet.size());

//        addr += size;
//        rem -= size;
//    }

//    return v;
//}

//uint16_t CncCom::read16(uint32_t addr) {
//    vector<uint8_t> v = read(addr, sizeof(uint16_t));
//    return BitConverter::toUInt16(v, 0);
//}

//uint32_t CncCom::read32(uint32_t addr) {
//    vector<uint8_t> v = read(addr, sizeof(uint32_t));
//    return BitConverter::toUInt32(v, 0);
//}

//uint64_t CncCom::read64(uint32_t addr) {
//    vector<uint8_t> v = read(addr, sizeof(uint64_t));
//    return BitConverter::toUInt64(v, 0);
//}

//uint64_t CncCom::read48(uint32_t addr) {
//    vector<uint8_t> v = read(addr, sizeof(uint16_t) * 3);
//    return BitConverter::toUInt48(v, 0);
//}

//vector<uint32_t> CncCom::readArray32(uint32_t addr, size_t len) {
//    vector<uint32_t> data(len);
//    vector<uint8_t> bytes = read(addr, len * sizeof(uint32_t));
//    memcpy(data.data(), bytes.data(), bytes.size());
//    return data;
//}
