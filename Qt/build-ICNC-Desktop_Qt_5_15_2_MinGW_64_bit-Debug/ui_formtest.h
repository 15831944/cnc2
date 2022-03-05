/********************************************************************************
** Form generated from reading UI file 'formtest.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORMTEST_H
#define UI_FORMTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FormTest
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTextEdit *txtMsg;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnHome;
    QPushButton *btnQuickTest;
    QPushButton *btnFullTest;
    QPushButton *btnImit;
    QPushButton *btnReadGCode;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_10;
    QPushButton *pushButton_11;
    QPushButton *pushButton_12;
    QPushButton *btnGCode;
    QPushButton *btnHelp;

    void setupUi(QWidget *FormTest)
    {
        if (FormTest->objectName().isEmpty())
            FormTest->setObjectName(QString::fromUtf8("FormTest"));
        FormTest->resize(1253, 633);
        verticalLayout_2 = new QVBoxLayout(FormTest);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        txtMsg = new QTextEdit(FormTest);
        txtMsg->setObjectName(QString::fromUtf8("txtMsg"));
        txtMsg->setReadOnly(true);

        verticalLayout->addWidget(txtMsg);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        btnHome = new QPushButton(FormTest);
        btnHome->setObjectName(QString::fromUtf8("btnHome"));

        horizontalLayout->addWidget(btnHome);

        btnQuickTest = new QPushButton(FormTest);
        btnQuickTest->setObjectName(QString::fromUtf8("btnQuickTest"));

        horizontalLayout->addWidget(btnQuickTest);

        btnFullTest = new QPushButton(FormTest);
        btnFullTest->setObjectName(QString::fromUtf8("btnFullTest"));

        horizontalLayout->addWidget(btnFullTest);

        btnImit = new QPushButton(FormTest);
        btnImit->setObjectName(QString::fromUtf8("btnImit"));

        horizontalLayout->addWidget(btnImit);

        btnReadGCode = new QPushButton(FormTest);
        btnReadGCode->setObjectName(QString::fromUtf8("btnReadGCode"));

        horizontalLayout->addWidget(btnReadGCode);

        pushButton_6 = new QPushButton(FormTest);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout->addWidget(pushButton_6);

        pushButton_7 = new QPushButton(FormTest);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        horizontalLayout->addWidget(pushButton_7);

        pushButton_8 = new QPushButton(FormTest);
        pushButton_8->setObjectName(QString::fromUtf8("pushButton_8"));

        horizontalLayout->addWidget(pushButton_8);

        pushButton_9 = new QPushButton(FormTest);
        pushButton_9->setObjectName(QString::fromUtf8("pushButton_9"));

        horizontalLayout->addWidget(pushButton_9);

        pushButton_10 = new QPushButton(FormTest);
        pushButton_10->setObjectName(QString::fromUtf8("pushButton_10"));

        horizontalLayout->addWidget(pushButton_10);

        pushButton_11 = new QPushButton(FormTest);
        pushButton_11->setObjectName(QString::fromUtf8("pushButton_11"));

        horizontalLayout->addWidget(pushButton_11);

        pushButton_12 = new QPushButton(FormTest);
        pushButton_12->setObjectName(QString::fromUtf8("pushButton_12"));

        horizontalLayout->addWidget(pushButton_12);

        btnGCode = new QPushButton(FormTest);
        btnGCode->setObjectName(QString::fromUtf8("btnGCode"));

        horizontalLayout->addWidget(btnGCode);

        btnHelp = new QPushButton(FormTest);
        btnHelp->setObjectName(QString::fromUtf8("btnHelp"));

        horizontalLayout->addWidget(btnHelp);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(FormTest);

        QMetaObject::connectSlotsByName(FormTest);
    } // setupUi

    void retranslateUi(QWidget *FormTest)
    {
        FormTest->setWindowTitle(QCoreApplication::translate("FormTest", "Form", nullptr));
        btnHome->setText(QCoreApplication::translate("FormTest", "Home", nullptr));
        btnQuickTest->setText(QCoreApplication::translate("FormTest", "Quick Test", nullptr));
        btnFullTest->setText(QCoreApplication::translate("FormTest", "Full Test", nullptr));
        btnImit->setText(QCoreApplication::translate("FormTest", "Imitation", nullptr));
        btnReadGCode->setText(QCoreApplication::translate("FormTest", "Read CNC", nullptr));
        pushButton_6->setText(QString());
        pushButton_7->setText(QString());
        pushButton_8->setText(QString());
        pushButton_9->setText(QString());
        pushButton_10->setText(QString());
        pushButton_11->setText(QString());
        pushButton_12->setText(QString());
        btnGCode->setText(QCoreApplication::translate("FormTest", "G-code", nullptr));
        btnHelp->setText(QCoreApplication::translate("FormTest", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FormTest: public Ui_FormTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORMTEST_H
