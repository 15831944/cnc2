#include "form_test.h"
#include "ui_formtest.h"

#include <QDebug>
#include <string>
#include <iostream>
#include <sstream>
#include <QFile>

#include "cnc.h"
#include "aux_items.h"

using namespace std;
using namespace auxItems;

const string job_dir = "c:\\Work\\Meatec\\Prototype\\NC\\";

FormTest::FormTest(ProgramParam &par, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTest),
    par(par),
    m_report(ui->txtMsg)
{
    ui->setupUi(this);
}

FormTest::~FormTest()
{
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void FormTest::on_btnHome_clicked()
{
    emit homePageClicked();
}

void FormTest::on_btnQuickTest_clicked()
{
    m_report.clear();
    par.cnc.bindReporter(&m_report);

    string s;

    if (par.cnc.isOpen()) {
        try {
            s = par.cnc.readVersion();
            m_report.write(s);
        } catch (string& s) {
            qDebug("%s\n", s.c_str());
        } catch (exception& e) {
            qDebug("Exception: %s\n", e.what());
        } catch (...) {
            qDebug("Unknown exception\n");
        }
    }
    else
        m_report.write("No CNC connection");
}

void FormTest::on_btnFullTest_clicked()
{
    m_report.clear();
    par.cnc.bindReporter(&m_report);

    string s;

    if (par.cnc.isOpen()) {
        try {
            s = par.cnc.readVersion();
            m_report.write(s);

            if (par.cnc.testRegs())
                ui->txtMsg->append("Info: RW registor OK");
            else
                ui->txtMsg->append("Error: RW registor error");

            if (par.cnc.testProgArray())
                ui->txtMsg->append("Info: RW program memory OK");
            else
                ui->txtMsg->append("Error: RW program memory error");

            if (par.cnc.testFpga())
                ui->txtMsg->append("Info: FPGA test OK");
            else
                ui->txtMsg->append("Error: FPGA test error");
        } catch (string& s) {
            qDebug("%s\n", s.c_str());
        } catch (exception& e) {
            qDebug("Exception: %s\n", e.what());
        } catch (...) {
            qDebug("Unknown exception\n");
            throw;
        }
    }
    else
        m_report.write("No CNC connection");
}

void FormTest::on_btnGCode_clicked()
{
    emit editPageClicked();
}

void FormTest::on_btnImit_clicked()
{
    m_report.clear();
    par.cnc.bindReporter(&m_report);

    if (par.gcodeText.length() == 0) {
        m_report.write(tr("Error: No G-code program") + "\n");
        return;
    }

    list<string> gframes = par.gcode.toFrameList();
    if (gframes.empty()) {
        m_report.write(tr("Error: No G-code program") + "\n");
        return;
    }

    try {
        string s = par.cnc.readVersion();
        m_report.write(s);

        if (par.cnc.testRegs())
            ui->txtMsg->append("Info: RW registor OK");
        else {
            ui->txtMsg->append("Error: RW registor error");
            return;
        }

        par.cnc.write(gframes);
        list<string> read_gframes = par.cnc.read();

        string error_text;
        bool OK = GCode::CompareFrames(gframes, read_gframes, error_text);

        if (OK) {
            string txt = GCode::toText(read_gframes);
            ui->txtMsg->append(QString(txt.c_str()));
        }
        else {
            ui->txtMsg->append(QString(error_text.c_str()));
            return;
        }

        if (OK) {
            par.cnc.imitEna(true);
            par.cnc.runReq();
            par.cnc.readImitFifo();
            par.cnc.saveImitData(job_dir + "motor_xy.dat");

//            system(("python " + job_dir + "test_plot1.py").c_str());
            int res = system("python c:\\Work\\Meatec\\Prototype\\NC\\test_plot1.py");
            qDebug() << res;
//            FILE* in = popen(("python " + job_dir + "test_plot1.py").c_str(), "r");
//            FILE* in = popen("python c:\\Work\\Meatec\\Prototype\\NC\\test_plot1.py", "r");
//            pclose(in);
        }
    } catch (string& s) {
        qDebug("%s\n", s.c_str());
    } catch (exception& e) {
        qDebug("Exception: %s\n", e.what());
    } catch (...) {
        qDebug("Unknown exception\n");
        throw;
    }
}

void FormTest::on_btnReadGCode_clicked() {
    list<string> read_gframes = par.cnc.read();
    string txt = GCode::toText(read_gframes);

    if (txt.empty())
        ui->txtMsg->setPlainText("No G-code program in CNC\n");
    else
        ui->txtMsg->setPlainText(QString(txt.c_str()));

    QFile file("readGcode.log");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(txt.c_str());
        file.close();
    }
}
