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
    _bonjourBrowser->setInterval(_browseInterval);
    connect(_bonjourBrowser,  SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)), this, SLOT(updateRecords(const QList<BonjourRecord> &)));
    connect(_bonjourBrowser,  SIGNAL(error(DNSServiceErrorType)),                 _bonjourBrowser,      SLOT(handleError(DNSServiceErrorType)));
    connect(_bonjourResolver, SIGNAL(error(DNSServiceErrorType)),                 _bonjourResolver,     SLOT(handleError(DNSServiceErrorType)));
    connect(_bonjourBrowser,  SIGNAL(finished()),                                 this,                 SLOT(startRecordResolve()));
    connect(_bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &)),   this,                 SLOT(saveHostInformation(QHostInfo)));
    connect(_bonjourResolver, SIGNAL(cleanedUp()),                                this,                 SLOT(prepareForNextRecord())); // TODO not sure with the timing here, maybe c
                                                                                                                                       // cleanup should wait until save is done
                                                                                                                                       // -> two signals to wait for: cleanedUp() and hostInfoSaved()
    connect(this,             SIGNAL(finished()),                                 this,                 SLOT(displayResults()));
}

Client::~Client()
{

}

int Client::start()
{
    QTimer::singleShot(_exploreInterval, this, SLOT(checkResults()));
    _bonjourBrowser->browseForServiceType(QLatin1String("_services._dns-sd._udp"));
    return 0;
}

void Client::saveHostInformation(const QHostInfo &hostInfo)
{
    BonjourRecord r = _allRecords.first();
    QMap<QString, QVariant> results;
    results.insert("registeredType", r.registeredType);
    results.insert("replyDomain", r.replyDomain);
    results.insert("serviceName", r.serviceName);
    results.insert("hostname", hostInfo.hostName());
    results.insert("lookupId", hostInfo.lookupId());

    const QList<QHostAddress> &addresses = hostInfo.addresses();
    foreach (QHostAddress addr, addresses) {
        results.insertMulti("ip", addr.toString());
    }
    _results.insertMulti(r.registeredType, results);
    emit hostInfoSaved();
}

void Client::checkResults()
{
    if(!_bonjourBrowser->_bonjourRecords.isEmpty())
    {
        _bonjourBrowser->cleanUp();
        _bonjourBrowser->browseForFoundServiceTypes();
    }else{
        qDebug() << "no devices found";
    }
}

void Client::startRecordResolve()
{
    _allRecords = _bonjourBrowser->_bonjourRecords;
    foreach(BonjourRecord b, _allRecords)
    {
        qDebug() << b.registeredType << b.replyDomain << b.serviceName;
    }

    getRecord();
}

void Client::getRecord()
{
    if(!_allRecords.isEmpty())
    {
        //TODO maybe get parameter for not searching for iphones->takes long, dns_sd maybe, never brings outcomes
//        if(_allRecords.first().registeredType == "_apple-mobdev2._tcp.") /*||
//           _allRecords.first().registeredType == "_dns-sd._udp.")
//        {
//            prepareForNextRecord();
//            return;
//        }
        _bonjourResolver->resolveBonjourRecord(_allRecords.first());

    }else{
        qDebug() << "Done";
        emit finished();
    }
}

void Client::prepareForNextRecord()
{
    _allRecords.removeFirst();
    getRecord();
}

void Client::displayResults()
{
    QStringList keys = _results.keys();
    keys = keys.toSet().toList();
    foreach(QString key, keys)
    {
        QList<QMap<QString, QVariant> > membersOfOneClass = _results.values(key);
        qDebug() << key << ":";
        QMap<QString, QVariant> m;
        foreach (m, membersOfOneClass) {
            qDebug() << "\tregisteredType:" << m.value("registeredType").toString();
            qDebug() << "\treplyDomain:"    << m.value("replyDomain").toString();
            qDebug() << "\tserviceName:"    << m.value("serviceName").toString();
            qDebug() << "\tIP:"             << m.value("ip").toString();
            qDebug() << "\thostname:"       << m.value("hostname").toString();
            qDebug() << "\tlookupId:"       << m.value("lookupId").toString();
            qDebug() << "";
        }
    }
    qDebug() << "Found a total of " << _results.size() << "elements in " << keys.length() << "different categories.";
    QCoreApplication::exit(0);
}

void Client::updateRecords(const QList<BonjourRecord> &list)
{
    foreach (BonjourRecord record, list)
    {
        if(record.registeredType == "_tcp." || record.registeredType == "_udp.")
        {
            //qDebug() << "handling more devices" << record.registeredType << record.replyDomain << record.serviceName;
        }else{
            //qDebug() << "Device:" << record.registeredType << record.replyDomain << record.serviceName;
        }
    }
}
