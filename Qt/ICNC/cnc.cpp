#include "cnc.h"
#include <vector>
#include <list>
#include "aux_items.h"
#include <cstdio>
#include <ctime>
#include <cmath>
//#include <chrono>
//#include <thread>
#include <unistd.h>
#include <QThread>
#include <QTextEdit>
#include <QDebug>
#include <QObject>
#include <QString>

using namespace std;
using namespace auxItems;

uint16_t CncParam::inputLevel = DEFAULT_INPUT_LEVEL;
bool CncParam::sdEnable = DEFAULT_SD_ENA;
bool CncParam::reverseX = false;
bool CncParam::reverseY = false;
bool CncParam::reverseU = false;
bool CncParam::reverseV = false;
bool CncParam::swapXY = false;
bool CncParam::swapUV = false;
bool CncParam::reverseEncX = false;
bool CncParam::reverseEncY = false;
double CncParam::step = DEFAULT_STEP; // mm
double CncParam::scaleX = DEFAULT_SCALE_XY, CncParam::scaleY = DEFAULT_SCALE_XY, CncParam::scaleU = DEFAULT_SCALE_UV, CncParam::scaleV = DEFAULT_SCALE_UV;
double CncParam::scaleEncX = DEFAULT_SCALE_ENC_XY, CncParam::scaleEncY = DEFAULT_SCALE_ENC_XY;
bool CncParam::encXY = false;

bool CncParam::fb_ena = false;
unsigned CncParam::rb_attempts = DEFAULT_RB_ATTEMPTS;
double CncParam::low_thld = 0, CncParam::high_thld = 200, CncParam::rb_to = DEFAULT_RB_TO, CncParam::rb_len = DEFAULT_RB_LEN, CncParam::rb_speed = DEFAULT_RB_SPEED;
double CncParam::acc = DEFAULT_ACC, CncParam::dec = DEFAULT_DEC;

CncCom Cnc::m_com;

Cnc::Cnc(QObject* parent) : QObject(parent), m_msg(nullptr), m_fpga(m_com) {}

Cnc::~Cnc() {
    close();
}

void Cnc::bindReporter(Reporter* const msg) {
    m_msg = msg;
}

void Cnc::unbindReporter() {
    m_msg = nullptr;
}

bool Cnc::open() {
    return m_com.open();
}

void Cnc::close() {
    reset();
    m_com.close();
}

bool Cnc::reset() {
    if (isOpen()) {
//        com.write32(ADDR::CLEAR, ADDR::RESET_MASK);
        return writeSettings(
            CncParam::inputLevel,
            false, false,
            CncParam::reverseX, CncParam::reverseY, CncParam::reverseU, CncParam::reverseV,
            CncParam::swapXY, CncParam::swapUV,
            CncParam::reverseEncX, CncParam::reverseEncY,
            100, 300
        ); // reset included
    }
    return false;
}

bool Cnc::writeProgArray(const vector<uint8_t>& bytes) {
    try {
        qDebug("Write to Program Array %d bytes\n", int(bytes.size()));
        if (isOpen()) {
            m_com.write(ADDR::PA, bytes);
            m_com.write32(ADDR::PA_RDADDR, 0);
            m_com.write32(ADDR::PA_WRADDR, static_cast<uint32_t>(bytes.size()));
            return true;
        }
    } catch (...) {}

    return false;
}

vector<uint8_t> Cnc::readProgArray() {
    qDebug("Read Program Array\n");
    uint32_t len = isOpen() ? m_com.read32(ADDR::PA_WRADDR) : 0;

    vector<uint8_t> bytes;
    if (isOpen())
        bytes = m_com.read(ADDR::PA, len);

    qDebug("read %d bytes\n", int(bytes.size()));
    return bytes;
}

string fileReadLine(FILE* fp) {
    string s;
    s.resize(256);
    s.clear();

    while (!feof(fp)) {
        int ch = fgetc(fp);

        if (ferror(fp)) {
            qDebug("Read file error\n");
            s.clear();
            break;
        }
        else {
            if (ch == '\n' || ch == '\0')
                break;
            else
                s += static_cast<char>(ch);
        }
    }

    return s;
}

string textReadLine(const string& text,  string::const_iterator& it) {
    string s;
    s.resize(256);
    s.clear();

    while (it != text.end()) {
        if (*it == '\n' || *it == '\0')
            break;
        else
            s += *it;

        ++it;
    }

    return s;
}

bool Cnc::writeFromFile(const string& fileName) {
    FILE* fp = fopen(fileName.c_str(), "r");

    if (fp == nullptr) {
        m_msg->write( QString::asprintf("File \"%s\" is not found\n", fileName.c_str()) );
        return false;
    }
    m_msg->write( QString::asprintf("Read G-code from file \"%s\"\n", fileName.c_str()) );

    list<string> ss;

    string s = fileReadLine(fp);
    size_t n = s.size() + 1;

    while (!s.empty()) {
        ss.push_back(std::move(s));
        s = fileReadLine(fp);
        n += s.size() + 1;
    }

    print_strings(m_msg, ss);

    vector<uint8_t> data;
    for (list<string>::const_iterator it = ss.begin(); it != ss.end(); ++it)
        push_back_range(data, *it);

    uint32_t pa_size = isOpen() ? m_com.read32(ADDR::PA_SIZE) : 0;

    if (data.size() > pa_size) {
        qDebug("G-code size is too big %d bytes (available only %d bytes)", int(data.size()), int(pa_size));
        return false;
    }

    return writeProgArray(data);
}

bool Cnc::write(const std::list<std::string>& frames) {
    vector<uint8_t> data;
    for (list<string>::const_iterator it = frames.begin(); it != frames.end(); ++it)
        push_back_range(data, *it);

    uint32_t pa_size = isOpen() ? m_com.read32(ADDR::PA_SIZE) : 0;

    if (data.size() > pa_size) {
        m_msg->write( QString::asprintf("G-code size is too big %d bytes (available only %d bytes)", int(data.size()), int(pa_size)) );
        return false;
    }

    return writeProgArray(data);
}

void Cnc::printGCode() {
    list<string> frames = read();
    print_strings(frames);
}

list<string> Cnc::read() {
    vector<uint8_t> bytes = readProgArray();
    list<string> frames;

    string s;
    for (size_t i = 0; i < bytes.size(); i++) {
        if (bytes[i] == '\0') {
            frames.push_back(s);
            s.clear();
        }
        else
            s += static_cast<char>(bytes[i]);
    }

    return frames;
}

#define FPGA_CLOCK (72e6)
/* scale - steps/mm
 * speed - mm/min
*/
bool Cnc::directMoveOn(int32_t nx, double scale_x, int32_t ny, double scale_y, int32_t nu, double scale_u, int32_t nv, double scale_v, double speed) {
    if (nx || ny || nu || nv) {
        try {            
            if (scale_x <= 0) scale_x = 0;
            if (scale_y <= 0) scale_y = 0;
            if (scale_u <= 0) scale_u = 0;
            if (scale_v <= 0) scale_v = 0;
            speed = fabs(speed);

            if (isOpen()) {
                double dx = nx / scale_x; // mm
                double dy = ny / scale_y;
                double du = nu / scale_u;
                double dv = nv / scale_v;

                double len_xy = sqrt(dx * dx + dy * dy);
                double len_uv = sqrt(du * du + dv * dv);
                double len = len_xy > len_uv ? len_xy : len_uv;

                double T = 60.0 * FPGA_CLOCK / speed; // clocks/mm

                double ts = len * T;

                int32_t Tx = nx ? auxItems::double_to_int32(ts / fabs(nx)) : 0;
                int32_t Ty = ny ? auxItems::double_to_int32(ts / fabs(ny)) : 0;
                int32_t Tu = nu ? auxItems::double_to_int32(ts / fabs(nu)) : 0;
                int32_t Tv = nv ? auxItems::double_to_int32(ts / fabs(nv)) : 0;

                int32_t data[] = {nx, Tx, ny, Ty, nu, Tu, nv, Tv};
                m_com.write(ADDR::MOVE_X, data, sizeof(data));
                return true;
            }
        } catch (...) {}

        return false;
    }

    return true;
}

bool Cnc::directSetPos(int32_t nx, int32_t ny, int32_t nu, int32_t nv, int32_t enc_x, int32_t enc_y) {
    try {
        if (isOpen()) {
            int32_t data[] = {nx, ny, nu, nv, enc_x, enc_y};
            m_com.write(ADDR::SET_X, data, sizeof(data));
            return true;
        }
    } catch (...) {}

    return false;
}

float angle360(float a) {
    if (a < 0)
        a = 0;
    else if (a > 2*M_PI)
        a = 2*M_PI;

    return a;
}

bool Cnc::centering(
        CENTER_MODE_T mode,
        int touches, int attempts,
        int drumVel, int thld,
        int fineSharePct, float R,
        float rollback,
        float speedCoarse, float speedFine,
        float angle0, float angle1, float angle2)
{
    if (touches < 1)
        touches = 1;
    else if (touches > CENTER_ATTEMPTS_MAX)
        touches = CENTER_ATTEMPTS_MAX;

    if (attempts < 1)
        attempts = 1;
    else if (attempts > CENTER_ATTEMPTS_MAX)
        attempts = CENTER_ATTEMPTS_MAX;

   if (drumVel < 0)
        drumVel = 0;
    else if (drumVel > 7)
        drumVel = 7;

   if (fineSharePct < 0)
       fineSharePct = 0;
   else if (fineSharePct > 100)
       fineSharePct = 100;

    if (R < -CENTER_RADIUS_MAX)
        R = -CENTER_RADIUS_MAX;
    else if (R > CENTER_RADIUS_MAX)
        R = CENTER_RADIUS_MAX;

    rollback = fabs(rollback);
    if (rollback > CENTER_RADIUS_MAX)
        rollback = CENTER_RADIUS_MAX;

    speedCoarse = fabs(speedCoarse);
    if (speedCoarse > SPEED_MAX) speedCoarse = SPEED_MAX;

    speedFine = fabs(speedFine);
    if (speedFine > SPEED_MAX) speedFine = SPEED_MAX;

    angle0 = angle360(angle0);
    angle1 = angle360(angle1);
    angle2 = angle360(angle2);

    if (mode == CENTER_MODE_T::CENTER_X || mode == CENTER_MODE_T::CENTER_Y || mode == CENTER_MODE_T::CENTER_CIRCLE_4R || mode == CENTER_MODE_T::CENTER_CIRCLE_3ADJ) {
        try {
            if (isOpen()) {
                if (R >= 0)
                    rollback = -rollback;

                int32_t data[] = {
                    int32_t(drumVel)<<24 | int32_t(attempts)<<16 | int32_t(touches)<<8 | int32_t(mode),
                    int32_t(fineSharePct)<<16 | int32_t(cnc_adc_volt_t::toCode(0, thld)),
                    *reinterpret_cast<int32_t*>(&R),
                    *reinterpret_cast<int32_t*>(&rollback),
                    *reinterpret_cast<int32_t*>(&speedCoarse),
                    *reinterpret_cast<int32_t*>(&speedFine),
                    *reinterpret_cast<int32_t*>(&angle0),
                    *reinterpret_cast<int32_t*>(&angle1),
                    *reinterpret_cast<int32_t*>(&angle2)
                };
                m_com.write(ADDR::CENTER_MODE, data, sizeof(data));
                return true;
            }
        } catch (...) {}
    }
    else if (mode == CENTER_MODE_T::CENTER_NO)
        return true;

    return false;
}

bool Cnc::runReq() {
    try {
        if (isOpen()) {
            m_com.write32(ADDR::STATUS, ADDR::RUN_MASK);
            return true;
        }
    } catch (...) {}

    return false;
}
bool Cnc::revReq() {
    try {
        if (isOpen()) {
            m_com.write32(ADDR::STATUS, ADDR::REV_MASK | ADDR::RUN_MASK);
            return true;
        }
    } catch (...) {}

    return false;
}

bool Cnc::cancelReq() {
    try {
        if (isOpen()) {
            m_com.write32(ADDR::CLEAR, ADDR::ABORT_MASK);
            return true;
        }
    } catch (...) {}

    return false;
}
bool Cnc::stopReq() {
    try {
        if (isOpen()) {
            m_com.write32(ADDR::CLEAR, ADDR::STOP_MASK);
            return true;
        }
    }
    catch (...) {}

    return false;
}

bool Cnc::isRun() {
    try {
        return isOpen() ? (m_com.read32(ADDR::STATUS) & ADDR::RUN_MASK) != 0 : false;
    } catch (...) {}

    return false;
}

cnc_context_t Cnc::readCncContext() const {
    vector<uint8_t> v;

    try {
        if (isOpen()) {
            v = m_com.read(ADDR::CONTEXT, CncContext::SIZE);
            cnc_context_t ctx = CncContext::parse(v);
            ctx.field.backup_valid = true; // add valid for save param
            return ctx;
        }
    } catch (...) { }

    return CncContext::defaultContext();
}

double Cnc::readDiameter() const {
    double res = 0;

    try {
        if (isOpen()) {
            vector<uint8_t> v = m_com.read(ADDR::DIA, sizeof(uint32_t));
            res = BitConverter::toFloat(v, 0);
        }
    } catch (...) { }

    return res;
}

cnc_context_t Cnc::readBackup() const {
    vector<uint8_t> v;

    try {
        if (isOpen()) {
            v = m_com.read(ADDR::BACKUP, CncContext::BACKUP_SIZE);
            return CncContext::parse(v);
        }
    } catch (...) { }

    return CncContext::defaultContext();
}

void Cnc::clearBackup() {
    try {
        if (isOpen())
            m_com.write32(ADDR::BACKUP + ((CncContext::BACKUP_SIZE32 - 1) << 2), 0);
    } catch (...) { }
}

// program must be loaded + reset
void Cnc::initialContext(const cnc_context_t& ctx) {
    writeXYUVEnc(ctx.field.x, ctx.field.y, ctx.field.u, ctx.field.v, ctx.field.enc_x, ctx.field.enc_y);
    writeGoto(ctx.field.id);

//    writePumpEnable(ctx.field.pump_ena);
//    writeDrumEnable(ctx.field.drum_state == unsigned(drum_state_t::DRUM_ANY) ||
//                    ctx.field.drum_state == unsigned(drum_state_t::DRUM_REV) ||
//                    ctx.field.drum_state == unsigned(drum_state_t::DRUM_REV)
//                    );
    writeWireEnable(ctx.field.wire_ena);
//    writeVoltageEnable(ctx.field.voltage_ena);
    writeHoldEnable(ctx.field.hold_ena);
    writeDrumVel(ctx.field.drum_vel);
    writeVolgateLevel(ctx.field.voltage_level);
    writeCurrentIndex(ctx.field.current_index);
    writePulseWidth(ctx.field.pulse_width);
    writePulseRatio(ctx.field.pulse_ratio);
    writeSpeed( WireSpeed::TtoSpeed(ctx.field.T) );
    writeEnableUV(ctx.field.uv_ena);
}

cnc_adc_t Cnc::readADC() const {
    vector<uint8_t> v;

    try {
        if (isOpen()) {
            v = m_com.read(ADDR::ADC, cnc_adc_t::SIZE);
            return cnc_adc_t::parse(v);
        }
    } catch (...) { }

    return cnc_adc_t();
}

cnc_adc_volt_t Cnc::readADCVolt() const {
    return cnc_adc_volt_t( readADC() );
}

void Cnc::imitEna(bool value) {
    if (isOpen()) {
        if (value)
            m_com.write32(ADDR::STATUS, ADDR::IMIT_MASK);
        else
            m_com.write32(ADDR::CLEAR, ADDR::IMIT_MASK);
    }
}

bool Cnc::isImitEna() {
    return isOpen() ? (m_com.read32(ADDR::STATUS) & ADDR::IMIT_MASK) != 0 : false;
}

void Cnc::readImitFifo() {
    m_imit_list.clear();

    int timeout = 10;
    int cnt = timeout;
    int pack_num = 0, total = 0;
    MotorRecord rec;
    const uint8_t max = 21 * MotorRecord::size; // len = 252

    do {
        timespec tic, toc;
        clock_gettime(CLOCK_MONOTONIC, &tic);

        vector<uint8_t> b;
        if (isOpen())
            b = m_com.readFifo(ADDR::IMIT_FIFO_Q, max);

        clock_gettime(CLOCK_MONOTONIC, &toc);
        double sec = 1.0 * (toc.tv_sec - tic.tv_sec) + 1e-9 * (toc.tv_nsec - tic.tv_nsec);

        cnt--;
        size_t it = 0;
        pack_num++;

        if (!b.empty()) {
            double baud = 8.0 * b.size() / sec;
            total += b.size();
            m_msg->write(QString::asprintf(\
                           "%d Read %g records (%d bytes). Total: records %g (%d bytes). Baud %g bit/s\n",\
                           pack_num, double(b.size()) / MotorRecord::size, int(b.size()), double(total) / MotorRecord::size, total, std::round(baud))\
                       );

            do {
                rec = MotorRecord(b, &it);
                m_imit_list.push_back(rec);
                if (rec.stop) break;
            } while (it < b.size());

            if (rec.stop) break;
            cnt = timeout;
        }
        else
            qDebug("%d Read %g records (%d bytes). Total: records %g (%d bytes). Baud %g bit/s\n", pack_num, 0.0, 0, double(total) / MotorRecord::size, total, 0.0);

//            if (b.size() < max / 2) usleep(100);
//            usleep(100);
        QThread::msleep(100);
    } while (cnt != 0);

//        if (!rec.stop)
//            qDebug("Timeout\n");
}

void Cnc::saveImitData(string fileName) {
    deque<int32_t> X, Y;

    for (deque<MotorRecord>::const_iterator it = m_imit_list.begin(); it != m_imit_list.end(); ++it)
        switch (it->axis) {
            case AXIS_X: X.push_back(it->N); Y.push_back(0); break;
            case AXIS_Y: X.push_back(0); Y.push_back(it->N); break;
            default: throw string("Axis error");
        }

    vector<int32_t> v;

    for (size_t i = 0; i < min(X.size(), Y.size()); i++) {
        v.push_back(X[i]);
        v.push_back(Y[i]);
    }

    FILE* fp = fopen(fileName.c_str(), "wb");

    if (fp == nullptr) {
        qDebug("File \"%s\" not found\n", fileName.c_str());
        return;
    }

    fwrite(v.data(), sizeof(int32_t), v.size(), fp);
    fflush(fp);
    m_msg->write("Imitation data saved to file \"" + fileName + "\"");
}

string Cnc::versionDate() {
    char date[4 * sizeof(uint32_t)], time[4 * sizeof(uint32_t)];
//        memset(date, 0, sizeof(date));
//        memset(time, 0, sizeof(time));
    uint32_t rddata;

    for (uint32_t i = 0; i < sizeof(date) / sizeof(uint32_t); i++) {
        rddata = isOpen() ? m_com.read32(ADDR::VER_DATE + (i << 2)) : 0;
        memcpy(&date[sizeof(uint32_t) * i], &rddata, sizeof(uint32_t));
    }

    for (uint32_t i = 0; i < sizeof(time) / sizeof(uint32_t); i++) {
        rddata = isOpen() ? m_com.read32(ADDR::VER_TIME + (i << 2)) : 0;
        memcpy(&time[sizeof(uint32_t) * i], &rddata, sizeof(uint32_t));
    }

    return string(date, 0, sizeof(date)) + " " + string(time, 0, sizeof(time));
}

uint32_t Cnc::sysClock() { return isOpen() ? m_com.read32(ADDR::SYS_CLOCK) : 0; }
cnc_version_t Cnc::version() { return isOpen() ? m_com.read32(ADDR::VER) : 0; }

string Cnc::readVersion() {
    cnc_version_t mcu_ver = version();
    string mcu_s = mcu_ver.toString();
    string mcu_date = versionDate();
    uint32_t f = sysClock();
    string res = QObject::tr("CNC").toStdString() + ": " + mcu_s;
    res += " " + QObject::tr("on").toStdString();
    res += string_format(" %s", mcu_date.c_str());
    res += " " + QObject::tr("at").toStdString();
    res += string_format(" %g", static_cast<double>(f/1e6));
    res += " " + QObject::tr("MHz").toStdString();
    qDebug("%s", res.c_str());
    return res;
}

bool Cnc::writeUInt32(uint32_t addr, uint32_t data) {
    try {
        if (isOpen()) {
            m_com.write32(addr, data);
            return true;
        }
    }
    catch (...) {}

    return false;
}

bool Cnc::writeFloat(uint32_t addr, float data) {
    const uint32_t* const ptr = reinterpret_cast<uint32_t*>(&data);

    try {
        if (isOpen()) {
            m_com.write32(addr, *ptr);
            return true;
        }
    }
    catch (...) {}

    return false;
}

bool Cnc::writeSetBits(uint32_t addr, unsigned bit, unsigned width, uint16_t data) {
    uint32_t mask;
    if (width >= 16)
        mask = 0xffff;
    else
        mask = (1<<width) - 1;

    mask <<= bit;

    uint32_t data32 = (mask<<16) | (uint16_t(data<<bit) & mask);

    return writeUInt32(addr, data32);
}

uint32_t Cnc::readUInt32(uint32_t addr, uint32_t defaultValue, bool* pOK) {
    try {
        if (isOpen()) {
            vector<uint8_t> v = m_com.read(addr, sizeof(uint32_t));
            if (pOK)
                *pOK = true;

            return BitConverter::toUInt32(v, 0);
        }
    } catch (...) {
        if (pOK)
            *pOK = false;
    }

    return defaultValue;
}

uint32_t Cnc::readBit(uint32_t addr, unsigned bit, unsigned width, uint32_t defaultValue, bool* pOK) {
    bool OK = false;
    uint32_t res = readUInt32(addr, 0, &OK);

    if (pOK)
        *pOK = OK;

    if (OK) {
        uint32_t mask;
        if (width > 31)
            mask = 0xffffffff;
        else
            mask = (1U<<width) - 1;

        return bit < 32 ? res>>bit & mask : 0;
    }

    return defaultValue;
}

bool Cnc::writePumpEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 0, 1, ena);
}
bool Cnc::writeDrumEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 1, 2, ena ? uint16_t(drum_state_t::DRUM_ANY) : uint16_t(drum_state_t::DRUM_DIS));
}
bool Cnc::writeWireEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 3, 1, ena);
}
// Enable high voltage if drum and pump enabled
// Disable high voltage, drum and pump
bool Cnc::writeVoltageEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 4, 1, ena);
}
bool Cnc::writeHoldEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 5, 1, ena);
}

bool Cnc::writeSemaphoreEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 6, 1, ena);
}

bool Cnc::writeSemaphoreCncEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 6, 2, ena ? 3 : 0);
}

bool Cnc::writeCncEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 7, 1, ena);
}

bool Cnc::writeWeakHVEnable(bool ena) {
    return writeSetBits(ADDR::CONTROLS_ENABLE, 8, 1, ena);
}

bool Cnc::writeEncXYEna(bool ena) {
    return writeSetBits(ADDR::BOOL_SET, 0, 1, ena);
}

bool Cnc::writeUVEna(bool ena) {
    return writeSetBits(ADDR::BOOL_SET, 8, 1, ena);
}

bool Cnc::writeDrumVel(unsigned value) { return writeUInt32(ADDR::DRUM_VEL, value); }
bool Cnc::writeVolgateLevel(unsigned value) { return writeUInt32(ADDR::VOLTAGE_LEVEL, value); }
bool Cnc::writeCurrentIndex(size_t index) {
    if (index > UINT8_MAX)
        index = UINT8_MAX;

    return writeUInt32(ADDR::CURRENT_INDEX, uint32_t(index));
}
bool Cnc::writePulseWidth(unsigned value) { return writeUInt32(ADDR::PULSE_WIDTH, value); }
bool Cnc::writePulseRatio(unsigned value) { return writeUInt32(ADDR::PULSE_RATIO, value); }

// value - mm/min
bool Cnc::writeSpeed(float value) { return writeFloat(ADDR::SPEED, value); }
bool Cnc::writeSpeed(const WireSpeed& value) { return writeSpeed( float(value.getMMM()) ); }

// mm
bool Cnc::writeStep(float value) { return writeFloat(ADDR::STEP, value); }

bool Cnc::writeEnableUV(bool ena) { return writeUInt32(ADDR::BOOL_SET, 1<<(16+8) | (uint32_t)ena<<8); }

bool Cnc::writeGoto(int32_t frame_num) {
    if (frame_num < 0)
        frame_num = 0;

    return writeUInt32(ADDR::GOTO, uint32_t(frame_num));
}
bool Cnc::writeXYUVEnc(int nx, int ny, int nu, int nv, int enc_x, int enc_y) {
    try {
        if (isOpen()) {
            int32_t data[] = {nx, ny, nu, nv, enc_x, enc_y};
            m_com.write(ADDR::REC_X, data, sizeof(data));
            return true;
        }
    } catch (...) {}

    return false;
}

bool Cnc::writeSettings(
        uint16_t input_lvl,
        bool sd_oe, bool sd_ena,
        bool rev_x, bool rev_y, bool rev_u, bool rev_v, bool swap_xy, bool swap_uv,
        bool rev_enc_x, bool rev_enc_y,
        double acc, double dec
        ) {
    try {
        if (isOpen()) {
            vector<uint8_t> v = vector<uint8_t>(5 * sizeof(uint32_t));

            uint32_t data = input_lvl;
            memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

            data = uint32_t(rev_enc_x)<<17 | uint32_t(rev_enc_y)<<16 |
                   uint32_t(swap_uv)<<9 | uint32_t(swap_xy)<<8 | uint32_t(rev_v)<<3 | uint32_t(rev_u)<<2 | uint32_t(rev_y)<<1 | uint32_t(rev_x);
            memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

            float fdata = acc; // per 100V
            memcpy(&v[8], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

            fdata = dec; // per 100V
            memcpy(&v[12], reinterpret_cast<uint8_t*>(&fdata), sizeof(float));

            data = uint32_t(sd_ena)<<1 | uint32_t(sd_oe);
            memcpy(&v[16], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

            m_com.write(ADDR::INPUT_LEVEL, v);

            return true;
        }
    } catch (...) {}

    qDebug("WRITE SETTINGS ERROR");
    return false;
}

bool Cnc::writeStep(float step, float scale_x, float scale_y, float scale_u, float scale_v, float scale_enc_x, float scale_enc_y, bool encXY) {
    try {
        if (isOpen()) {
            vector<uint8_t> v = vector<uint8_t>(8 * sizeof(float));

            memcpy(&v[0], reinterpret_cast<uint8_t*>(&step), sizeof(float));
            memcpy(&v[4], reinterpret_cast<uint8_t*>(&scale_x), sizeof(float));
            memcpy(&v[8], reinterpret_cast<uint8_t*>(&scale_y), sizeof(float));
            memcpy(&v[12], reinterpret_cast<uint8_t*>(&scale_u), sizeof(float));
            memcpy(&v[16], reinterpret_cast<uint8_t*>(&scale_v), sizeof(float));
            memcpy(&v[20], reinterpret_cast<uint8_t*>(&scale_enc_x), sizeof(float));
            memcpy(&v[24], reinterpret_cast<uint8_t*>(&scale_enc_y), sizeof(float));

            uint32_t data = 1<<16 | encXY<<0;
            memcpy(&v[28], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

            m_com.write(ADDR::STEP, v);

            return true;
        }
    } catch (...) {}

    qDebug("WRITE SETTINGS ERROR");
    return false;
}

bool Cnc::readSettings(
        uint16_t& input_lvl,
        bool& sd_oe, bool& sd_ena,
        bool& rev_x, bool& rev_y, bool& rev_u, bool& rev_v,
        bool& swap_xy, bool& swap_uv,
        bool& rev_enc_x, bool& rev_enc_y,
        double& acc, double& dec
        ) {
    try {
        if (isOpen()) {
            vector<uint8_t> v = m_com.read(ADDR::INPUT_LEVEL, 5 * sizeof(uint32_t));

            if (v.size() == 5 * sizeof(uint32_t)) {
                input_lvl = BitConverter::toUInt16(v, 0);

                uint32_t motor_dir = BitConverter::toUInt32(v, 4);
                rev_x = motor_dir & 1;
                rev_y = (motor_dir & 1<<1) != 0;
                rev_u = (motor_dir & 1<<2) != 0;
                rev_v = (motor_dir & 1<<3) != 0;
                swap_xy = (motor_dir & 1<<8) != 0;
                swap_uv = (motor_dir & 1<<9) != 0;
                rev_enc_x = (motor_dir & 1<<16) != 0;
                rev_enc_y = (motor_dir & 1<<17) != 0;

                acc = BitConverter::toFloat(v, 8); // per 100V
                dec = BitConverter::toFloat(v, 12); // per 100V

                uint32_t sd = BitConverter::toUInt32(v, 16);
                sd_oe = (sd & 1<<0) != 0;
                sd_ena = (sd & 1<<1) != 0;

                return true;
            }
        }
    } catch (...) {}

    qDebug("READ SETTINGS ERROR");
    return false;
}

bool Cnc::readStep(float &step, float &scaleX, float &scaleY, float &scaleU, float &scaleV, float &scaleEncX, float &scaleEncY, bool &encXY) {
    try {
        if (isOpen()) {
            const size_t size = 8 * sizeof(uint32_t);
            vector<uint8_t> v = m_com.read(ADDR::STEP, size);

            if (v.size() == size) {
                step = BitConverter::toUInt32(v, 0);
                scaleX = BitConverter::toUInt32(v, 4);
                scaleY = BitConverter::toUInt32(v, 8);
                scaleU = BitConverter::toUInt32(v, 12);
                scaleV = BitConverter::toUInt32(v, 16);
                scaleEncX = BitConverter::toUInt32(v, 20);
                scaleEncY = BitConverter::toUInt32(v, 24);
                encXY = v[28] & 1;

                return true;
            }
        }
    } catch (...) {}

    qDebug("READ SETTINGS ERROR");
    return false;
}

//bool Cnc::writeAdcThreshold(bool enable, uint16_t low, uint16_t high) {
//    if (high < low)
//        high = low;

//    //    CncParam::inputLevel = value;

//        try {
//            if (isOpen()) {
//                if (!enable)
//                    com.write32(ADDR::ADC_ENA, 0);
//                else
//                    com.write64(ADDR::ADC_THDL, static_cast<uint64_t>(enable)<<32 | static_cast<uint64_t>(high)<<16 | low);

//                return true;
//            }
//        }
//        catch (...) {}

//        qDebug("WRITE ADC THRESHOLD LEVEL ERROR");
//        return false;
//}

//bool Cnc::writeAdcThresholdVolt(bool enable, double low, double high) {
//    return writeAdcThreshold( enable, cnc_adc_volt_t::toCode(0, low), cnc_adc_volt_t::toCode(0, high) );
//}

//bool Cnc::readAdcThreshold(bool& enable, uint16_t& low, uint16_t& high) {
//    try {
//        if (isOpen()) {
//            uint64_t data = com.read64(ADDR::ADC_THDL);
//            low = static_cast<uint16_t>(data);
//            high = static_cast<uint16_t>(data>>16);
//            enable = (data & (1ULL << 32)) != 0;
////            CncParam::inputLevel = value;
//            return true;
//        }
//    } catch (...) {}

//    qDebug("WRITE ADC THRESHOLD LEVEL ERROR");
//    return false;
//}

//bool Cnc::readAdcThresholdVolt(bool& enable, double& low, double& high) {
//    uint16_t low_code, high_code;

//    bool OK = readAdcThreshold(enable, low_code, high_code);
//    low = cnc_adc_volt_t::toVolt(0, low_code);
//    high = cnc_adc_volt_t::toVolt(0, high_code);
//    return OK;
//}

bool Cnc::writeFeedback(bool enable, double Vlow, double Vhigh, double to_sec, uint32_t attempts, double length_mm, double speed_mmm) {
//    int Vthld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );
//    Vlow  = Vlow  > Vthld_max ? Vthld_max : Vlow;
//    Vhigh = Vhigh > Vthld_max ? Vthld_max : Vhigh;

    if (Vhigh < Vlow)
        Vhigh = Vlow;

    try {
        if (isOpen()) {
            if (!enable)
                m_com.write32(ADDR::FB_ENA, 0);
            else {
                vector<uint8_t> v = vector<uint8_t>(6 * sizeof(uint32_t));

                uint32_t data = enable;
                memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

                data = static_cast<uint32_t>(cnc_adc_volt_t::toCode(0, Vhigh))<<16 | cnc_adc_volt_t::toCode(0, Vlow);
                memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

                uint32_t ms = static_cast<uint32_t>(round(to_sec * 1e3));
                memcpy(&v[8], reinterpret_cast<uint8_t*>(&ms), sizeof(uint32_t));

                memcpy(&v[12], reinterpret_cast<uint8_t*>(&attempts), sizeof(uint32_t));

                float mm = length_mm;
                memcpy(&v[16], reinterpret_cast<uint8_t*>(&mm), sizeof(float));

                float mmm = speed_mmm;
                memcpy(&v[20], reinterpret_cast<uint8_t*>(&mmm), sizeof(float));

                m_com.write(ADDR::FB_ENA, v);
            }

            return true;
        }
    } catch (...) {}

    return false;
}

bool Cnc::writeFeedback(bool enable, double Vlow, double Vhigh) {
    if (Vhigh < Vlow)
        Vhigh = Vlow;

    try {
        if (isOpen()) {
            if (!enable)
                m_com.write32(ADDR::FB_ENA, 0);
            else {
                vector<uint8_t> v = vector<uint8_t>(6 * sizeof(uint32_t));

                uint32_t data = enable;
                memcpy(&v[0], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

                data = static_cast<uint32_t>(cnc_adc_volt_t::toCode(0, Vhigh))<<16 | cnc_adc_volt_t::toCode(0, Vlow);
                memcpy(&v[4], reinterpret_cast<uint8_t*>(&data), sizeof(uint32_t));

                m_com.write(ADDR::FB_ENA, v);
            }

            return true;
        }
    } catch (...) {}

    return false;
}

bool Cnc::readFeedback(bool &enable, double &Vlow, double &Vhigh, double &to_sec, uint32_t &attempts, double &length_mm, double &speed_mmm) {
    try {
        if (isOpen()) {
            vector<uint8_t> v = m_com.read(ADDR::FB_ENA, 6 * sizeof(uint32_t));

            if (v.size() == 6 * sizeof(uint32_t)) {
                enable = (v[0] & 1) != 0;
                Vlow = cnc_adc_volt_t::toVolt(0, BitConverter::toUInt16(v, 4));
                Vhigh = cnc_adc_volt_t::toVolt(0, BitConverter::toUInt16(v, 6));
                to_sec = BitConverter::toUInt32(v, 8) / 1e3;
                attempts = BitConverter::toUInt32(v, 12);
                length_mm = BitConverter::toFloat(v, 16);
                speed_mmm = BitConverter::toFloat(v, 20);
                return true;
            }
        }
    } catch (...) {}

    return false;
}
