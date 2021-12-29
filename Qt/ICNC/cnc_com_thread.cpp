#include "cnc_com_thread.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QCoreApplication>

#include <iostream>
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
//#include <regex>

#include "main.h"
#include "aux_items.h"

using namespace std;
using namespace auxItems;

CncComThread::CncComThread(QObject *parent): QThread(parent) {}

CncComThread::~CncComThread() {
    close();
}

bool CncComThread::open() {
    QString portName;

    // Print all serial porta and open the last suitable
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info: list) {
        if (!info.isNull()) {
            qDebug("name: %s PRODUCT_ID: %x VENDOR_ID: %x", info.portName().toStdString().c_str(), info.productIdentifier(), info.vendorIdentifier());

            if (info.productIdentifier() == ST_PRODUCT_ID && info.vendorIdentifier() == ST_VENDOR_ID) {
                portName = info.portName();
            }
        }
    }

    if (m_port) {
        if (m_port->isOpen())
            m_port->close();

        delete m_port;
        m_port = nullptr;
    }

    if (portName.size() != 0) {
        m_port = new QSerialPort();
        m_port->setPortName(portName);

        // uart port parameters need for correct work, but don't use
        m_port->setBaudRate(QSerialPort::Baud115200);
        m_port->setDataBits(QSerialPort::Data8);
        m_port->setParity(QSerialPort::NoParity);
        m_port->setStopBits(QSerialPort::OneStop);
        m_port->setFlowControl(QSerialPort::NoFlowControl);
        m_port->setReadBufferSize(16 * 1024);

        if (!m_port->open(QIODevice::ReadWrite)) {
            delete m_port;
            m_port = nullptr;
            m_message = QString::asprintf("Port %s is not opened", portName.toStdString().c_str()).toStdString();
        }
        else {
            m_message = QString::asprintf("Port %s is opened", portName.toStdString().c_str()).toStdString();
            // Start thread
            transaction(ComPacket());
        }

        return m_port != nullptr;
    }
    else {
        m_message = "STM USB CDC device was not found";
        return false;
    }
}

bool CncComThread::isOpen() {
    if (isRunning()) {
        m_mutex.lock();
        bool open = m_port && m_port->isOpen();
        m_mutex.unlock();
        return open && isRunning();
    }

    return m_port && m_port->isOpen();
}

void CncComThread::close() {
    if (isRunning()) {
        m_mutex.lock();
        m_quit = true;
        m_cond.wakeOne();
        m_mutex.unlock();
        wait();
    }
    m_quit = false;
    m_ready = false;

    if (m_port) {
        delete m_port;
        m_port = nullptr;
    }
}

bool CncComThread::asyncReady() {
    m_mutex.lock();
    bool open = m_port && m_port->isOpen();
    bool ready = m_ready;
    m_mutex.unlock();

    return open && isRunning() && ready;
}

int CncComThread::readPort(ComPacket &rxd, int timeout_ms) {
    rxd.clear();

    if (timeout_ms < 10) {
        timeout_ms = 10;
    }
    if (m_port) {
        do {
            bool OK = m_port->waitForReadyRead(timeout_ms);

            if (OK) {
                QByteArray responseData = m_port->readAll();

                while (m_port->waitForReadyRead(10))
                    responseData += m_port->readAll();


                for (QByteArray::iterator it = responseData.begin(); it != responseData.end(); ++it) {
                    rxd.append(*it);

                    if (rxd.complete())
                        break;
                }
            }
            else {
                rxd.finalize();
                return -2;
            }

        } while (!rxd.complete());

#ifdef RX_PRINT
        static int n;

        if (rxd.empty())
            n = 0;

        if (!rxd.empty()) {
            for (size_t i = 0; i < rxd.count(); i++) {
                if (n != 15)
                    qDebug("%02X ", int(rxd.rawData(i)));
                else
                    qDebug("%02X\n", int(rxd.rawData(i)));
            }
        }
        else {
            if (n != 15)
                qDebug("Empty packet ");
            else
                qDebug("Empty packet\n");
        }
        n++;
#endif
        return rxd.size();
    }
    else {
        rxd.finalize();
        return -1;
    }
}

void CncComThread::writeBytes(uint32_t addr, const std::vector<uint8_t> &bytes) {
    if (bytes.size()) {
        if ((addr & 0xF0000000) != 0)
            throw runtime_error( string_format("Address error: 0x%08x", int(addr)) );

        if (!m_port || !m_port->isOpen())
            throw runtime_error("COM port isn't opened");

        m_txpack.createWritePacket(addr, bytes, 0, bytes.size());

        m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()), qint64(m_txpack.rawSize()));
        m_port->flush();

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Write to address 0x%08x:\n", addr);
        auxItems::print_array(m_txpack.rawData(), m_txpack.rawSize());
#endif

        while (true) {
            int res = readPort(m_rxpack, TIMEOUT);

            if (res <= 0)
                throw runtime_error( string("Rx timeout. Received ") + to_string(m_rxpack.rawSize()) + " bytes" );

            if (m_rxpack.complete()) {
                if (m_rxpack.command() == ComPacket::Command::CMD_ERROR)
                    throw runtime_error("CNC sent an error code");

                if (m_rxpack.error()) {
                    switch (m_rxpack.error_code()) {
                    case 1: throw runtime_error("Write acknowledge. Unknown command");
                    case 2: throw runtime_error("Write acknowledge. CRC Error");
                    case 3: throw runtime_error("Write acknowledge. Timeout Error");
                    default: throw runtime_error("Write acknowledge. Unknown Error");
                    }
                }

                if (m_rxpack.command() != ComPacket::Command::CMD_WRITE)
                    throw runtime_error(string_format("Write acknowledge. Command error: 0x%x", int(m_rxpack.command())));

                if (m_rxpack.addr() != addr)
                    throw runtime_error(string_format("Write acknowledge. Address error: 0x%x", int(m_rxpack.addr())));

                if (m_rxpack.size() != bytes.size())
                    throw runtime_error(string_format("Write acknowledge. Length error: %d", int(m_rxpack.size())));

                break;
            }
        }
    }
}

void CncComThread::writeBytes(uint32_t addr, const uint8_t* data, size_t size, size_t begin, size_t length) {
    if (begin < size) {
        data += begin;

        if (begin + length > size)
            length = static_cast<uint8_t>(size - begin);

        if (length != 0) {
            vector<uint8_t> bytes(length);
            memcpy(bytes.data(), data, bytes.size());
            writeBytes(addr, bytes);
        }
    }
}

std::vector<uint8_t> CncComThread::readBytes(uint32_t addr, size_t length, ComPacket::Command cmd) {
    const QMutexLocker portLocker(&m_mutexPort);
    vector<uint8_t> rddata(length);

    if (length != 0 && (cmd == ComPacket::Command::CMD_READ || cmd == ComPacket::Command::CMD_READ_FIFO)) {
        if ((addr & 0xF0000000) != 0)
            throw runtime_error( string_format("Address error: 0x%08x", int(addr)) );

        if (!m_port || !m_port->isOpen())
            throw runtime_error("COM port isn't opened");

        m_txpack.createReadPacket(cmd, addr, length);

        m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()), qint64(m_txpack.rawSize()));
        m_port->flush();

#ifdef PRINT_CNC_COM_DEBUG
        qDebug("Write to address 0x%08x:\n", addr);
        auxItems::print_array(m_txpack.rawData(), m_txpack.rawSize());
#endif

        while (true) {
            int res = readPort(m_rxpack, TIMEOUT);

            if (res <= 0)
                throw runtime_error( string("Rx timeout. Received ") + to_string(m_rxpack.rawSize()) + " bytes" );

            if (m_rxpack.complete()) {
                if (m_rxpack.command() == ComPacket::Command::CMD_ERROR)
                    throw runtime_error("CNC sent an error code");

                if (m_rxpack.error()) {
                    switch (m_rxpack.error_code()) {
                    case 1: throw runtime_error("Write acknowledge. Unknown command");
                    case 2: throw runtime_error("Write acknowledge. CRC Error");
                    case 3: throw runtime_error("Write acknowledge. Timeout Error");
                    default: throw runtime_error("Write acknowledge. Unknown Error");
                    }
                }

                if (m_rxpack.command() != cmd)
                    throw runtime_error(string_format("Write acknowledge. Command Error: 0x%x", int(m_rxpack.command())));

                if (m_rxpack.addr() != addr)
                    throw runtime_error(string_format("Write acknowledge. Address Error: 0x%x", int(m_rxpack.addr())));

                if (m_rxpack.size() != length)
                    throw runtime_error(string_format("Write acknowledge. Length Error: %d", int(m_rxpack.size())));

                break;
            }
        }
    }

    memcpy(rddata.data(), m_rxpack.data(), rddata.size());
    return rddata;
}

// Start or wake up a transducer thread
void CncComThread::transaction(const ComPacket& request) {
    const QMutexLocker locker(&m_mutex);
    m_request = request;

    if (!isRunning())
        start();
    else
        m_cond.wakeOne();
}

void CncComThread::run() {
    ComPacket response;

    qDebug("CNC thread started");

    m_mutex.lock();
    ComPacket currentRequest = m_request;
    m_ready = false;
    m_mutex.unlock();    

    while (!m_quit) {
        if (!isOpen()) {
            qDebug("CNC thread is closing. Port is not opened");
            return;
        }

        if (!currentRequest.empty()) {
            const QMutexLocker portLocker(&m_mutexPort);

            // write request
            m_port->write(reinterpret_cast<const char*>(m_txpack.rawData()), qint64(currentRequest.rawSize()));
            m_port->flush();

            if (m_port->waitForBytesWritten(TIMEOUT)) {
                // read response
                while (true) {
                    if (m_port->waitForReadyRead(TIMEOUT)) {
                        QByteArray responseData = m_port->readAll();

                        while (m_port->waitForReadyRead(10))
                            responseData += m_port->readAll();

                        for (QByteArray::iterator it = responseData.begin(); it != responseData.end(); ++it) {
                            response.append(*it);

                            if (response.complete()) {
                                emit this->response(response);
                                response.clear();
                                break;
                            }
                        }
                    } else {
                        emit timeout(tr("Wait read response timeout"));
                        break;
                    }
                }
            } else {
                emit timeout(tr("Wait write request timeout"));
            }
        }

        m_mutex.lock();
        m_ready = true;
        qDebug("CNC thread is waiting");
        m_cond.wait(&m_mutex);
        m_ready = false;
        currentRequest = m_request;
        m_mutex.unlock();
    }

    qDebug("CNC thread is quit");
}

void CncComThread::write(uint32_t addr, const void* const data, size_t size) {
    const QMutexLocker portLocker(&m_mutexPort);
    const size_t max = ComPacket::MAX;

    size_t pos = 0;

    while (size > 0) {
        size_t len = size > max ? max : size;

        this->writeBytes(addr, reinterpret_cast<const uint8_t*>(data), size, pos, len);

        addr += len;
        pos += len;
        size -= len;
    }
}

void CncComThread::write(uint32_t addr, const vector<uint8_t>& bytes) {
    write(addr, bytes.data(), bytes.size());
}

// Data don't reverse
void CncComThread::write16(uint32_t addr, uint16_t data) {
    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
    this->write(addr, v);
}

void CncComThread::write32(uint32_t addr, uint32_t data) {
    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
    this->write(addr, v);
}

void CncComThread::write64(uint32_t addr, uint64_t data) {
    vector<uint8_t> v = vector<uint8_t>(sizeof(data));
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
    this->write(addr, v);
}

void CncComThread::write48(uint32_t addr, uint64_t data) {
    vector<uint8_t> v = vector<uint8_t>(sizeof(uint16_t) * 3);
    memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), v.size());
    this->write(addr, v);
}

vector<uint8_t> CncComThread::read(uint32_t addr, size_t len) {
    vector<uint8_t> v;
    const size_t max = ComPacket::MAX;
    size_t rem = len;

    while (rem > 0) {
        size_t size = rem > max ? max : static_cast<size_t>(rem);

        vector<uint8_t> packet = readBytes(addr, size);
        push_back_range(v, packet, 0, packet.size());

        addr += size;
        rem -= size;
    }

    return v;
}

std::vector<uint8_t> CncComThread::readFifo(uint32_t addr, size_t length) {
    return readBytes(addr, length, ComPacket::Command::CMD_READ_FIFO);
}

vector<uint32_t> CncComThread::readArray32(uint32_t addr, size_t len) {
    vector<uint32_t> data(len);
    vector<uint8_t> bytes = read(addr, len * sizeof(uint32_t));
    memcpy(data.data(), bytes.data(), bytes.size());
    return data;
}

uint16_t CncComThread::read16(uint32_t addr) {
    vector<uint8_t> v = read(addr, sizeof(uint16_t));
    return BitConverter::toUInt16(v, 0);
}

uint32_t CncComThread::read32(uint32_t addr) {
    vector<uint8_t> v = read(addr, sizeof(uint32_t));
    return BitConverter::toUInt32(v, 0);
}

uint64_t CncComThread::read64(uint32_t addr) {
    vector<uint8_t> v = read(addr, sizeof(uint64_t));
    return BitConverter::toUInt64(v, 0);
}

uint64_t CncComThread::read48(uint32_t addr) {
    vector<uint8_t> v = read(addr, sizeof(uint16_t) * 3);
    return BitConverter::toUInt48(v, 0);
}
