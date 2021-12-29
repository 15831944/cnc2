#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#define CNC_APP_MODEL ("0.2.2.19b")
// 0.2.2.1 изменен информационный пакет ЧПУ
// 0.2.2.2 попытка перевернуть оси в QWT
// 0.2.2.3 исправление ошибок Idle Run, Circle generator, Options->Write
// 0.2.2.4 debug feedback
// 0.2.2.8 Command keys in Code Editor, clear Run panel
// 0.2.2.0 Time AM PM for non-russian languages

//#define LINUX // - defined in pro file
#define STONE
//#define DEV
#ifndef STONE
    #define SHOW_ENCODERS
#endif
//#define TEST_ADC

#define X_SCALE (1000.0) // steps / mm
#define Y_SCALE (X_SCALE) // steps / mm
#define U_SCALE (1000.0) // steps / mm
#define V_SCALE (U_SCALE) // steps / mm
#define PRECISION (0.001/2) // +-0.5 um

#define X_ENC_SCALE (200.0)
#define Y_ENC_SCALE (200.0)

#ifdef LINUX
#define DEFAULT_COM ("/dev/ttyACM0")
#else
#define DEFAULT_COM ("COM6")
#endif

#define ST_VENDOR_ID (0x483)
#define ST_PRODUCT_ID (0x5740)

#define FULL_SCREEN

#if defined(STONE)
    #define CNC_APP_TYPE (" (" + tr("stone") + ")")
#else
    #define CNC_APP_TYPE ("")
#endif

#endif // CONSTRAINTS_H
