#ifndef CNCCOMASYNC_H
#define CNCCOMASYNC_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPort>
#include <vector>
#include <string>

#include "aux_items.h"
#include "com_packet.h"

#define PRINT_CNC_COM_DEBUG

// Class provides communication with CNC
class CncComThread : public QThread {
    Q_OBJECT

    static constexpr int TIMEOUT = 1000; // ms

    QSerialPort* m_port = nullptr;
    ComPacket m_txpack, m_rxpack;

    std::string m_message;
    ComPacket m_request;

    QMutex m_mutex, m_mutexPort;
    QWaitCondition m_cond;
    bool m_quit = false, m_ready = false;

public:
    explicit CncComThread(QObject *parent = nullptr);
    ~CncComThread();

    //    bool open(const std::string& portName); // Linux: "/dev/ttyACM0" || Win: "COM6"
    bool open(); // open use STM vendor ID
    bool isOpen();
    void close();
    bool asyncReady();

private:
    int readPort(ComPacket& rxd, int timeout_ms = 30000);
    void writeBytes(uint32_t addr, const std::vector<uint8_t>& bytes);
    void writeBytes(uint32_t addr, const uint8_t* bytes, size_t size, size_t begin, size_t length);
    std::vector<uint8_t> readBytes(uint32_t addr, size_t length, ComPacket::Command command = ComPacket::Command::CMD_READ);
    void run() override;

public:
    void write(uint32_t addr, const void* const data, size_t size);
    void write(uint32_t addr, const std::vector<uint8_t>& bytes);
    void write16(uint32_t addr, uint16_t data);
    void write32(uint32_t addr, uint32_t data);
    void write48(uint32_t addr, uint64_t data);
    void write64(uint32_t addr, uint64_t data);

    std::vector<uint8_t> read(uint32_t addr, size_t len);
    std::vector<uint8_t> readFifo(uint32_t addr, size_t length);
    std::vector<uint32_t> readArray32(uint32_t addr, size_t len);
    uint16_t read16(uint32_t addr);
    uint32_t read32(uint32_t addr);
    uint64_t read48(uint32_t addr);
    uint64_t read64(uint32_t addr);

protected:
    void transaction(const ComPacket& request);

    inline std::string message() {
        std::string s;
        s.swap(m_message);
        return s;
    }

signals:
    void response(const ComPacket& rxpack);
    void error(const QString& s);
    void timeout(const QString& s);
};

#endif // CNCCOMASYNC_H
