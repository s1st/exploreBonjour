/****************************************************************************
**
** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "client.h"

#include <QMessageBox>
#include <QtNetwork>
#include <QTimer>

Client::Client()
{
    _bonjourBrowser = new BonjourServiceBrowser(this);
    _bonjourResolver = new BonjourServiceResolver(this);
    allRecords = new QList<QVariant>;
    _counter = 0;
    connect(_bonjourBrowser,    SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
    connect(_bonjourResolver,   SIGNAL(bonjourRecordResolved(const QHostInfo &)),
            this, SLOT(listHosts(QHostInfo)));
    connect(_bonjourBrowser,    SIGNAL(error(DNSServiceErrorType)),                             _bonjourBrowser,    SLOT(handleError(DNSServiceErrorType)));
    connect(_bonjourResolver,   SIGNAL(error(DNSServiceErrorType)),                             _bonjourResolver,   SLOT(handleError(DNSServiceErrorType)));
    connect(this,               SIGNAL(foundDevice(QString,BonjourRecord)),                     this,               SLOT(handleDevice(QString, BonjourRecord)));
    connect(this,               &Client::foundDeviceClass,                                      this,               &Client::handleDeviceClass);
    connect(this,               &Client::hostFound,                                             this,               &Client::checkResults);
}

Client::~Client()
{

}

int Client::start()
{
    QTimer::singleShot(_interval, this, SLOT(checkResults()));

    _bonjourBrowser->browseForServiceType(QLatin1String("_services._dns-sd._udp"));
//    _bonjourBrowser->browseForServiceType(QLatin1String("_workstation._tcp"));
//    _bonjourBrowser->browseForServiceType(QLatin1String("_trollfortune._tcp"));
//    _bonjourBrowser->browseForServiceType(QLatin1String("_apple-mobdev2._tcp"));
//    _bonjourBrowser->browseForServiceType(QLatin1String("_googlecast._tcp"));
//    _bonjourBrowser->browseForServiceType(QLatin1String("_raop._tcp"));
    return 0;
}

void Client::listHosts(const QHostInfo &hostInfo)
{
    const QList<QHostAddress> &addresses = hostInfo.addresses();
    QString name = hostInfo.localHostName();
    qDebug() << "Done\nfound these hosts - " << name << ":";
    foreach (QHostAddress addr, addresses) {
        qDebug() << addr.toString();
    }
    _counter++;
    emit hostFound();
}

void Client::checkResults()
{
    qDebug() << "results after" << _interval << "milliseconds";
    if(!_bonjourBrowser->bonjourRecords.isEmpty())
    {
        _bonjourBrowser->cleanUp();
        _bonjourBrowser->browseForFoundServiceTypes(_counter);
    }
}

void Client::handleDevice(QString device, BonjourRecord br)
{
    qDebug() << "Handling and Resolving:" << device << br.registeredType << br.replyDomain << br.serviceName;
    //TODO handling centralized, again after 5 sec or so
//    _bonjourResolver->resolveBonjourRecord(br);
}

void Client::handleDeviceClass(QString deviceClass)
{
    qDebug() << "handling more devices" << deviceClass;
//    _bonjourBrowser->browseForServiceType(deviceClass);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "The host was not found. Please check the "
                                    "host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct.";
        break;
    default:
        qDebug() << "The following error occurred: %1."
                                 << tcpSocket->errorString();
    }
}

void Client::updateRecords(const QList<BonjourRecord> &list)
{
    QList<QVariant> vars;
    foreach (BonjourRecord record, list) {
        QVariant variant;
        variant.setValue(record);
        vars.append(variant);
        allRecords->append(variant);
        if(record.registeredType == "_tcp." || record.registeredType == "_udp.")
        {
            emit foundDeviceClass(record.serviceName
                                  .append(".")
                                  .append(record.registeredType));
        }else{
            emit foundDevice(record.serviceName
                             .append(".")
                             .append(record.registeredType), record);
        }
    }
//    foreach(QVariant variant, vars)
//    {
//        //TODO do not start form the beginning all the time
//    if(variant.value<BonjourRecord>().registeredType == "_tcp." || variant.value<BonjourRecord>().registeredType == "_udp.")
//    {
//        emit foundDeviceClass(variant.value<BonjourRecord>().serviceName
//                              .append(".")
//                              .append(variant.value<BonjourRecord>().registeredType));
//    }else{
//        emit foundDevice(variant.value<BonjourRecord>().serviceName
//                         .append(".")
//                         .append(variant.value<BonjourRecord>().registeredType), variant.value<BonjourRecord>());
//    }
//    }
}
