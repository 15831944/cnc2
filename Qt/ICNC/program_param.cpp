#include "program_param.h"
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <string>

InterfaceLanguage ProgramParam::lang = InterfaceLanguage::ENGLISH;
int ProgramParam::fontSize = 10;
bool ProgramParam::swapXY = false;
bool ProgramParam::reverseX = false;
bool ProgramParam::reverseY = false;
bool ProgramParam::showXY = false;

ProgramParam::ProgramParam() {
//    m_SettingsFile = QApplication::applicationDirPath() + "/settings.ini";
//    qDebug() << m_SettingsFile;
    setDefaultGenModeList();
    loadSettings();
//    cnc.bindCncParam(&cncParam);
}

ProgramParam::~ProgramParam() {
    saveSettings();
}

void ProgramParam::loadSettings() {
//    QSettings settings(m_SettingsFile, QSettings::NativeFormat);
    QSettings settings(org, app);
    fileDir = settings.value("fileDir", "").toString();
    gcodeFileName = settings.value("cncFileName", "").toString();
    contourFileName = settings.value("contourFileName", "").toString();
}

void ProgramParam::saveSettings() {
//    QSettings settings(m_SettingsFile, QSettings::NativeFormat);
    QSettings settings(org, app);
    settings.setValue("fileDir", fileDir);
    settings.setValue("cncFileName", gcodeFileName);
    settings.setValue("contourFileName", contourFileName);
    //    settings.sync();
}

void ProgramParam::saveCncContext() {
    QSettings settings(org, app);
    QByteArray bytes = cncContext.getByteArray();
    settings.setValue("cnc_context_t",  bytes);
}

void ProgramParam::clearCncContext() {
    cncContext.setValid(false);
    saveCncContext();
}

void ProgramParam::loadCncContext() {
    QSettings settings(org, app);

    if (!settings.contains("cnc_context_t"))
        cncContext.setDefault();
    else {
        QByteArray bytes = settings.value("cnc_context_t", QByteArray()).toByteArray();
        cncContext.set(bytes);
    }
}

// save G-code as contex
void ProgramParam::saveGcode() {
    QSettings settings(org, app);
    clearCncContext();
    settings.setValue("gcodeCnc", gcodeCnc);
//    saveInputLevel(CncParam::inputLevel);
}

QString ProgramParam::loadGcode() {
    QSettings settings(org, app);
    bool OK;

    loadInputLevel(OK);

    if (OK)
        return settings.value("gcodeCnc", QString()).toString();

    return QString();
}

void ProgramParam::setDefaultGenModeList() {
    genModesList.clear();
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
    genModesList.push_back(GeneratorMode());
}

void ProgramParam::setDefaultCutParam() {
    cut = cut_t();
}

// Parse a text file into the binary structure
GCode ProgramParam::compile(const QString& txt) {
    using namespace std;

    GCode gcode;
    int err = gcode.parse(txt.toStdString());

    if (err == 0) {
        string new_txt = gcode.toText();
        qDebug() << new_txt.c_str();
    }
    else
        gcode.clear();

    return gcode;
}

void ProgramParam::loadBackup() {
    loadCncContext();

    QString txt = loadGcode();
    gcode = compile(txt);

    if (gcode.empty())
        clearCncContext();
    else {
        gcodeSettings = gcode.getSettings();
        gcodeText = gcode.toText().c_str();
    }
}

void ProgramParam::saveInterfaceLanguage(InterfaceLanguage lang) {
    QSettings settings(org, app);
    settings.setValue("language", static_cast<uint8_t>(lang));
    settings.sync();
}

InterfaceLanguage ProgramParam::loadInterfaceLanguage() {
    QSettings settings(org, app);

    unsigned res = settings.value("language", static_cast<unsigned>(InterfaceLanguage::ENGLISH)).toUInt();

    if (res < static_cast<unsigned>(InterfaceLanguage::END))
        lang = static_cast<InterfaceLanguage>(res);
    else
        lang = InterfaceLanguage::ENGLISH;

    return lang;
}

QString ProgramParam::helpSubDir() {
#if defined(STONE)
    switch (lang) {
    case InterfaceLanguage::ENGLISH: return "stone/en";
    case InterfaceLanguage::RUSSIAN: return "stone/ru";
    default: return "stone/en";
    }
#else
    switch (lang) {
    case InterfaceLanguage::ENGLISH: return "metal/en";
    case InterfaceLanguage::RUSSIAN: return "metal/ru";
    default: return "metal/en";
    }
#endif
}

void ProgramParam::saveFontSize(int fontSize) {
    QSettings settings(org, app);
    ProgramParam::fontSize = fontSize;
    settings.setValue("fontSize", static_cast<uint8_t>(fontSize));
    settings.sync();
}

int ProgramParam::loadFontSize() {
    QSettings settings(org, app);

    int res = settings.value("fontSize", 10).toInt();

    if (res >= 8 && res <= 32)
        fontSize = res;
    else
        fontSize = 10;

    return fontSize;
}

void ProgramParam::saveInputLevel(uint16_t value) {
    QSettings settings(org, app);
    CncParam::inputLevel = value;
    settings.setValue("inputLevel", CncParam::inputLevel);
    settings.sync();
}

uint16_t ProgramParam::loadInputLevel(bool& OK) {
    QSettings settings(org, app);
    OK = settings.contains("inputLevel");
    CncParam::inputLevel = static_cast<uint16_t>(settings.value("inputLevel", CncParam::DEFAULT_INPUT_LEVEL).toUInt() & 0xFFFF);
    return CncParam::inputLevel;
}

void ProgramParam::saveStepDir(bool sdEna) {
    QSettings settings(org, app);
    CncParam::sdEnable = sdEna;
    settings.setValue("sdEnable", CncParam::sdEnable);
    settings.sync();
}

bool ProgramParam::loadStepDir(bool &OK) {
    QSettings settings(org, app);
    OK = settings.contains("sdEnable");
    CncParam::sdEnable = settings.value("sdEnable", CncParam::DEFAULT_SD_ENA).toBool();
    return CncParam::sdEnable;
}

void ProgramParam::saveMotorDir(bool revX, bool revY, bool revU, bool revV, bool swapXY, bool swapUV, bool reverseEncX, bool reverseEncY) {
    QSettings settings(org, app);
    CncParam::reverseX = revX;
    CncParam::reverseY = revY;
    CncParam::reverseU = revU;
    CncParam::reverseV = revV;
    CncParam::swapXY = swapXY;
    CncParam::swapUV = swapUV;
    CncParam::reverseEncX = reverseEncX;
    CncParam::reverseEncY = reverseEncY;

    settings.setValue("motorReverseX", CncParam::reverseX);
    settings.setValue("motorReverseY", CncParam::reverseY);
    settings.setValue("motorReverseU", CncParam::reverseU);
    settings.setValue("motorReverseV", CncParam::reverseV);
    settings.setValue("motorSwapXY", CncParam::swapXY);
    settings.setValue("motorSwapUV", CncParam::swapUV);
    settings.setValue("encReverseX", CncParam::reverseEncX);
    settings.setValue("encReverseY", CncParam::reverseEncY);
    settings.sync();
}

void ProgramParam::saveFeedbackParam(bool fb_ena, double low_thld, double high_thld, double rb_to, unsigned rb_attempts, double rb_len, double rb_speed) {
    QSettings settings(org, app);
    CncParam::fb_ena = fb_ena;
    CncParam::low_thld = low_thld;
    CncParam::high_thld = high_thld;
    CncParam::rb_to = rb_to;
    CncParam::rb_attempts = rb_attempts;
    CncParam::rb_len = rb_len;
    CncParam::rb_speed = rb_speed;

    settings.setValue("feedbackEnable", CncParam::fb_ena);
    settings.setValue("lowThreshold", CncParam::low_thld);
    settings.setValue("highThreshold", CncParam::high_thld);
    settings.setValue("rollbackTimeout", CncParam::rb_to);
    settings.setValue("rollbackAttempts", CncParam::rb_attempts);
    settings.setValue("rollbackLength", CncParam::rb_len);
    settings.setValue("rollbackSpeed", CncParam::rb_speed);
    settings.sync();
}

void ProgramParam::saveAcceleration(double acc, double dec) {
    QSettings settings(org, app);
    CncParam::acc = acc;
    CncParam::dec = dec;

    settings.setValue("acceleration", CncParam::acc);
    settings.setValue("deceleration", CncParam::dec);
    settings.sync();
}

void ProgramParam::saveStep(double step, double scaleX, double scaleY, double scaleU, double scaleV, double scaleEncX, double scaleEncY, bool encXY) {
    QSettings settings(org, app);
    CncParam::step = step;
    CncParam::scaleX = scaleX;
    CncParam::scaleY = scaleY;
    CncParam::scaleU = scaleU;
    CncParam::scaleV = scaleV;
    CncParam::scaleEncX = scaleEncX;
    CncParam::scaleEncY = scaleEncY;
    CncParam::encXY = encXY;

    settings.setValue("step", CncParam::step);
    settings.setValue("scaleX", CncParam::scaleX);
    settings.setValue("scaleY", CncParam::scaleY);
    settings.setValue("scaleU", CncParam::scaleU);
    settings.setValue("scaleV", CncParam::scaleV);
    settings.setValue("scaleEncX", CncParam::scaleEncX);
    settings.setValue("scaleEncY", CncParam::scaleEncY);
    settings.setValue("encXY", CncParam::encXY);

    settings.sync();
}

bool ProgramParam::loadMotorReverseX() {
    QSettings settings(org, app);
    return CncParam::reverseX = settings.value("motorReverseX", false).toBool();
}

bool ProgramParam::loadMotorReverseY() {
    QSettings settings(org, app);
    return CncParam::reverseY = settings.value("motorReverseY", false).toBool();
}

bool ProgramParam::loadMotorReverseU() {
    QSettings settings(org, app);
    return CncParam::reverseU = settings.value("motorReverseU", false).toBool();
}

bool ProgramParam::loadMotorReverseV() {
    QSettings settings(org, app);
    return CncParam::reverseV = settings.value("motorReverseV", false).toBool();
}

bool ProgramParam::loadMotorSwapXY() {
    QSettings settings(org, app);
    return CncParam::swapXY = settings.value("motorSwapXY", false).toBool();
}

bool ProgramParam::loadMotorSwapUV() {
    QSettings settings(org, app);
    return CncParam::swapUV = settings.value("motorSwapUV", false).toBool();
}

bool ProgramParam::loadEncReverseX() {
    QSettings settings(org, app);
    return CncParam::reverseEncX = settings.value("encReverseX", false).toBool();
}

bool ProgramParam::loadEncReverseY() {
    QSettings settings(org, app);
    return CncParam::reverseEncY = settings.value("encReverseY", false).toBool();
}

void ProgramParam::loadFeedbackParam(bool& fb_ena, double& low_thld, double& high_thld, double& rb_to, unsigned& rb_attempts, double& rb_len, double& rb_speed) {
    QSettings settings(org, app);
    fb_ena = CncParam::fb_ena = settings.value("feedbackEnable", false).toBool();
    low_thld = CncParam::low_thld = settings.value("lowThreshold", 0).toDouble();

    int thld_max = static_cast<int>( round(cnc_adc_volt_t::maxVolt(0)) );
    high_thld = CncParam::high_thld = settings.value("highThreshold", thld_max).toDouble();

    rb_to = CncParam::rb_to = settings.value("rollbackTimeout", CncParam::DEFAULT_RB_TO).toDouble();
    rb_attempts = CncParam::rb_attempts = settings.value("rollbackAttempts", CncParam::DEFAULT_RB_ATTEMPTS).toDouble();
    rb_len = CncParam::rb_len = settings.value("rollbackLength", CncParam::DEFAULT_RB_LEN).toDouble();
    rb_speed = CncParam::rb_speed = settings.value("rollbackSpeed", CncParam::DEFAULT_RB_SPEED).toDouble();
}

void ProgramParam::loadAcceleration(double &acc, double &dec) {
    QSettings settings(org, app);
    acc = CncParam::acc = settings.value("acceleration", CncParam::DEFAULT_ACC).toDouble();
    dec = CncParam::dec = settings.value("deceleration", CncParam::DEFAULT_DEC).toDouble();
}

void ProgramParam::loadStep(double &step, double &scaleX, double &scaleY, double &scaleU, double &scaleV, double &scaleEncX, double &scaleEncY, bool &encXY) {
    QSettings settings(org, app);
    step        = CncParam::step        = settings.value("step",        CncParam::DEFAULT_STEP).toDouble();
    scaleX      = CncParam::scaleX      = settings.value("scaleX",      CncParam::DEFAULT_SCALE_XY).toDouble();
    scaleY      = CncParam::scaleY      = settings.value("scaleY",      CncParam::DEFAULT_SCALE_XY).toDouble();
    scaleU      = CncParam::scaleU      = settings.value("scaleU",      CncParam::DEFAULT_SCALE_UV).toDouble();
    scaleV      = CncParam::scaleV      = settings.value("scaleV",      CncParam::DEFAULT_SCALE_UV).toDouble();
    scaleEncX   = CncParam::scaleEncX   = settings.value("scaleEncX",   CncParam::DEFAULT_SCALE_ENC_XY).toDouble();
    scaleEncY   = CncParam::scaleEncY   = settings.value("scaleEncY",   CncParam::DEFAULT_SCALE_ENC_XY).toDouble();
    encXY       = CncParam::encXY       = settings.value("encXY",       CncParam::DEFAULT_ENC_XY).toBool();
}

void ProgramParam::loadParam() {
    bool OK, fb_ena;
    unsigned rb_attempts;
    double low_thld, high_thld, rb_to, rb_len, rb_speed, acc, dec;
    double step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY;
    bool encXY;

    loadInterfaceLanguage();
    loadFontSize();

    loadSwapXY();
    loadReverseX();
    loadReverseY();
    loadShowXY();

    loadInputLevel(OK);

    loadMotorReverseX();
    loadMotorReverseY();
    loadMotorReverseU();
    loadMotorReverseV();
    loadMotorSwapXY();
    loadMotorSwapUV();

    loadEncReverseX();
    loadEncReverseY();

    loadFeedbackParam(fb_ena, low_thld, high_thld, rb_to, rb_attempts, rb_len, rb_speed);
    loadAcceleration(acc, dec);
    loadStep(step, scaleX, scaleY, scaleU, scaleV, scaleEncX, scaleEncY, encXY);
}

//
void ProgramParam::saveSwapXY(bool value) {
    QSettings settings(org, app);
    swapXY = value;
    settings.setValue("swapXY", swapXY);
    settings.sync();
}

bool ProgramParam::loadSwapXY() {
    QSettings settings(org, app);
    return swapXY = settings.value("swapXY", false).toBool();
}

void ProgramParam::saveReverseX(bool value) {
    QSettings settings(org, app);
    reverseX = value;
    settings.setValue("reverseX", reverseX);
    settings.sync();
}

bool ProgramParam::loadReverseX() {
    QSettings settings(org, app);
    return reverseX = settings.value("reverseX", false).toBool();
}

void ProgramParam::saveReverseY(bool value) {
    QSettings settings(org, app);
    reverseY = value;
    settings.setValue("reverseY", reverseY);
    settings.sync();
}

bool ProgramParam::loadReverseY() {
    QSettings settings(org, app);
    return reverseY = settings.value("reverseY", false).toBool();
}

void ProgramParam::saveShowXY(bool value) {
    QSettings settings(org, app);
    showXY = value;
    settings.setValue("showXY", showXY);
    settings.sync();
}

bool ProgramParam::loadShowXY() {
    QSettings settings(org, app);
    return showXY = settings.value("showXY", false).toBool();
}

//enum BUTTON_T {START, REVERSE, CANCEL};
//enum CNC_STATES_T {UNCHECKED, IDLE, RUN, PAUSE, REVERSE, CANCEL, SHORT_REVERSE, ERROR};

void AppState::next(AppState::BUTTON btn) {
    switch (m_state) {
    case STATES::ST_NONE: case STATES::ST_IDLE:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        break;
    case STATES::ST_RUN:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_PAUSE;
        break;
    case STATES::ST_PAUSE:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        else if (btn == BUTTON::SIG_REVERSE)
            m_state = STATES::ST_REV;
        else if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_CANCEL;
        break;
    case STATES::ST_REV:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_RUN;
        break;
    case STATES::ST_CANCEL:
        if (btn == BUTTON::SIG_REVERSE)
            m_state = STATES::ST_SHORT_REV;
        else if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_NONE;
        break;
    case STATES::ST_SHORT_REV:
        if (btn == BUTTON::SIG_START)
            m_state = STATES::ST_CANCEL;
        break;
    case STATES::ST_ERROR:
        if (btn == BUTTON::SIG_CANCEL)
            m_state = STATES::ST_NONE;
        break;
    default:
        m_state = STATES::ST_ERROR;
        break;
    }
}
