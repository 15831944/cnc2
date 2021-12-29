//#ifndef CNCCOM_H
//#define CNCCOM_H

//#include <QSerialPort>
//#include <QSerialPortInfo>
//#include <QTimer>
//#include <vector>
//#include <string>
//#include "aux_func.h"
//#include "com_packet.h"

//#define PRINT_CNC_COM_DEBUG

//// Class provides communication with CNC
//class CncCom: QObject {
//    Q_OBJECT

//    static constexpr int TIMEOUT = 1000; // ms
//    static constexpr uint8_t READ = 1;
//    static constexpr uint8_t WRITE = 2;
//    uint8_t m_state = 0;

//    QSerialPort* m_port = nullptr;
//    QTimer* timer = nullptr;
//    ComPacket txd, rxd;
//    aux_func::ReportWriter txt; // print debug messages to GUI

//    int readPort(ComPacket& rxd, int timeout_ms = 30000);

//public:
//    CncCom(aux_func::ReportWriter txt);
//    ~CncCom();

////    bool open(const std::string& portName); // Linux: "/dev/ttyACM0" || Win: "COM6"
//    bool open(); // open use STM vendor ID
//    inline bool isOpen() {
//        return m_port && m_port->isOpen();
//    }
//    bool close();

//private:
//    void writeBytesAsync(uint32_t addr, const std::vector<uint8_t>& bytes, size_t begin, size_t length);
//    void readBytesAsync(uint32_t addr, size_t length, ComPacket::Command command = ComPacket::Command::CMD_READ);

//    void writeBytes(uint32_t addr, const std::vector<uint8_t>& bytes, size_t begin, size_t length);
//    void writeBytes(uint32_t addr, const uint8_t* bytes, size_t size, size_t begin, size_t length);

//    std::vector<uint8_t> readBytes(uint32_t addr, size_t length, ComPacket::Command command = ComPacket::Command::CMD_READ);

//public:
//    void write(uint32_t addr, const void* data, size_t size);
//    void write(uint32_t addr, const std::vector<uint8_t>& bytes);
//    void write16(uint32_t addr, uint16_t data);
//    void write32(uint32_t addr, uint32_t data);
//    void write48(uint32_t addr, uint64_t data);
//    void write64(uint32_t addr, uint64_t data);

//    std::vector<uint8_t> read(uint32_t addr, size_t len);
//    uint16_t read16(uint32_t addr);
//    uint32_t read32(uint32_t addr);
//    uint64_t read48(uint32_t addr);
//    uint64_t read64(uint32_t addr);

//    std::vector<uint32_t> readArray32(uint32_t addr, size_t len);

//    std::vector<uint8_t> readFifo(uint32_t addr, size_t length);

//private slots:
//    void onReadyData();
//    void onBytesWritten();
//    void onTimeOut();

//signals:
//    void response(const ComPacket& rxd);
//};

//#endif // CNCCOM_H
