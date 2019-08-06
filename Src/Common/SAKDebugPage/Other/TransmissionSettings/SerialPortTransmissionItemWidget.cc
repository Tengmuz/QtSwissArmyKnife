﻿/*
 * Copyright (C) 2019 wuuhii. All rights reserved.
 *
 * The file is encoding with utf-8 (with BOM). It is a part of QtSwissArmyKnife
 * project. The project is a open source project, you can get the source from:
 *     https://github.com/wuuhii/QtSwissArmyKnife
 *     https://gitee.com/wuuhii/QtSwissArmyKnife
 *
 * If you want to know more about the project, please join our QQ group(952218522).
 * In addition, the email address of the project author is wuuhii@outlook.com.
 * Welcome to bother.
 *
 * I write the comment in English, it's not because that I'm good at English,
 * but for "installing B".
 */
#include "SAKCommonApi.hh"
#include "SerialPortTransmissionItemWidget.hh"
#include "ui_SerialPortTransmissionItemWidget.h"

#include <QSerialPort>
#include <QSerialPortInfo>

SerialPortTransmissionItemWidget::SerialPortTransmissionItemWidget(SAKDebugPage *debugPage, QWidget *parent)
    :BaseTransmissionItemWidget (debugPage, parent)
    ,ui (new Ui::SerialPortTransmissionItemWidget)
{
    ui->setupUi(this);

    enableCheckBox              = ui->enableCheckBox;
    handleReceiveDataCheckBox   = ui->handleReceiveDataCheckBox;
    comComboBox                 = ui->comComboBox;
    customBaudrateCheckBox      = ui->customBaudrateCheckBox;
    baudRateComboBox            = ui->baudRateComboBox;
    dataBitscomboBox            = ui->dataBitscomboBox;
    stopBitscomboBox            = ui->stopBitscomboBox;
    parityComboBox              = ui->parityComboBox;

    SAKCommonApi::instance()->initComComboBox(comComboBox);
    SAKCommonApi::instance()->initBaudRateComboBox(baudRateComboBox);
    SAKCommonApi::instance()->initDataBitsComboBox(dataBitscomboBox);
    SAKCommonApi::instance()->initStopBitsComboBox(stopBitscomboBox);
    SAKCommonApi::instance()->initParityComboBox(parityComboBox);

    handleReceiveDataCheckBox->setChecked(true);
}

void SerialPortTransmissionItemWidget::on_enableCheckBox_clicked()
{
    setTransmissionEnable(enableCheckBox->isChecked());
}

void SerialPortTransmissionItemWidget::on_handleReceiveDataCheckBox_clicked()
{
    setEnableHandleReceivedData(handleReceiveDataCheckBox->isChecked());
}

void SerialPortTransmissionItemWidget::on_customBaudrateCheckBox_clicked()
{
    baudRateComboBox->setEditable(customBaudrateCheckBox->isChecked());
}
