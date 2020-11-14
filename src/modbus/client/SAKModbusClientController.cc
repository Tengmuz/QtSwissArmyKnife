﻿/*
 * Copyright 2020 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */
#include <QDebug>
#include <QDateTime>
#include <QTabWidget>
#include <QFileDialog>
#include <QTableWidget>
#include <QModbusTcpServer>
#include <QModbusRtuSerialMaster>

#include "SAKModbusClientController.hh"
#include "SAKModbusCommonClientSection.hh"

SAKModbusClientController::SAKModbusClientController(QWidget *parent)
    :SAKModbusCommonController(parent)
    ,mClientSection(Q_NULLPTR)
{
    mModbusServer = new QModbusTcpServer(this);
    setModbusServerMap(mModbusServer);
    mClientSection = new SAKModbusCommonClientSection(this);
    connect(mClientSection, &SAKModbusCommonClientSection::invokeSendReadRequest, this, &SAKModbusClientController::sendReadRequest);
    connect(mClientSection, &SAKModbusCommonClientSection::invokrSendWriteRequest, this, &SAKModbusClientController::sendWriteRequest);
}

void SAKModbusClientController::setData(QModbusDataUnit::RegisterType type, quint16 address, quint16 value)
{
    if ((type == QModbusDataUnit::Coils) || (type == QModbusDataUnit::DiscreteInputs)){
        mModbusServer->setData(type, address, value ? true : false);
    }else {
        mModbusServer->setData(type, address, value);
    }
}

quint16 SAKModbusClientController::registerValue(QModbusDataUnit::RegisterType type, quint16 address)
{
    quint16 value = 0;
    if (!mModbusServer->data(type, address, &value)){
        qWarning() << "Can not get the value of register which type is:" << type;
    }

    return value;
}

void SAKModbusClientController::exportRegisterData()
{
    auto fileName = getSaveFileName();
    if (fileName.length()){
        saveServerRegisterData(mModbusServer, fileName);
    }
}

void SAKModbusClientController::importRegisterData()
{
    auto fileName = getOpenFileName();
    if (fileName.length()){
        setServerRegisterData(mModbusServer, fileName);
    }
}

bool SAKModbusClientController::tempData(QModbusDataUnit::RegisterType table, quint16 address, quint16 *data)
{
    if (data){
        mModbusServer->data(table, address, data);
        return true;
    }else{
        return false;
    }
}

QWidget *SAKModbusClientController::bottomSection()
{
    return mClientSection;
}

void SAKModbusClientController::sendReadRequest(QModbusDataUnit mdu)
{
    auto reply = qobject_cast<QModbusClient*>(device())->sendReadRequest(mdu, mClientSection->slaveAddress());
    if (reply){
        emit modbusDataUnitWritten(mdu);
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &SAKModbusClientController::readReply);
        } else {
            delete reply; // broadcast replies return immediately
        }
    }else{
        qWarning() << "Read error:" << qobject_cast<QModbusClient*>(device())->errorString();
    }
}

void SAKModbusClientController::sendWriteRequest(QModbusDataUnit mdu)
{
    auto reply = qobject_cast<QModbusClient*>(device())->sendWriteRequest(mdu, mClientSection->slaveAddress());
    if (reply) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    qWarning() << tr("Write response error: %1 (Mobus exception: 0x%2)").arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16);
                } else if (reply->error() != QModbusDevice::NoError) {
                    qWarning() << tr("Write response error: %1 (code: 0x%2)").arg(reply->errorString()).arg(reply->error(), -1, 16);
                }else{
                    emit modbusDataUnitWritten(mdu);
                }

                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        qWarning() << tr("Write error: ") + qobject_cast<QModbusClient*>(device())->errorString();
    }
}

void SAKModbusClientController::readReply()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply){
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit mdu = reply->result();
        mClientSection->updateTableWidget(mdu);
        emit modbusDataUnitRead(mdu);

        auto registerType = reply->result().registerType();
        auto startAddress = reply->result().startAddress();
        auto registerNumber = reply->result().valueCount();
        for (uint i = 0; i < mdu.valueCount(); i++) {
            auto value = reply->result().value(startAddress + i);
            setData(registerType, startAddress + i, value);
#ifdef QT_DEBUG
            const QString entry = tr("Address: %1, Value: %2").arg(mdu.startAddress() + i).arg(QString::number(mdu.value(i), mdu.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            qDebug() << entry;
#endif
        }
        // The signal is using to update ui value.
        emit dataWritten(registerType, startAddress, registerNumber);
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16);
    } else {
        qDebug() << tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16);
    }

    reply->deleteLater();
}