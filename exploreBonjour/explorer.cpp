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

#include "explorer.h"

#include <QtNetwork>
#include <QTimer>

Explorer::Explorer()
{
    _bonjourBrowser = new BonjourServiceBrowser(this);
    _bonjourResolver = new BonjourServiceResolver(this);
    _bonjourBrowser->setInterval(_browseInterval);
    connect(_bonjourBrowser,  SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)), this, SLOT(updateRecords(const QList<BonjourRecord> &)));
    connect(_bonjourBrowser,  SIGNAL(error(DNSServiceErrorType)),                 _bonjourBrowser,      SLOT(handleError(DNSServiceErrorType)));
    connect(_bonjourResolver, SIGNAL(error(DNSServiceErrorType)),                 _bonjourResolver,     SLOT(handleError(DNSServiceErrorType)));
    connect(_bonjourBrowser,  SIGNAL(finished()),                                 this,                 SLOT(startRecordResolve()));
    connect(_bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &)),   this,                 SLOT(saveHostInformation(QHostInfo)));
    connect(_bonjourResolver, SIGNAL(cleanedUp()),                                this,                 SLOT(prepareForNextRecord()));
    //connect(this,             SIGNAL(finished()),                                 this,                 SLOT(displayResults()));
    connect(this,             SIGNAL(finished()),                                 this,                 SLOT(saveResultsToDB()));
}

Explorer::~Explorer()
{

}

/*
 * Starts a timer after which the results of a browsing are checked.
 * The browse for "_services._dns-sd._udp" returns all service types in a network.
 */

int Explorer::start()
{
//    saveResultsToDB();
    QTimer::singleShot(_exploreInterval, this, SLOT(checkResults()));
    _bonjourBrowser->browseForServiceType(QLatin1String("_services._dns-sd._udp"));
    return 0;
}

/*
 * Saves all found hosts in a QMultiMap<QString, QMap<QString, QVariant> >
 * with the servicetype as key in the QMultiMap
 */

void Explorer::saveHostInformation(const QHostInfo &hostInfo)
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

/*
 * Checks found servicetypes
 */

void Explorer::checkResults()
{
    if(!_bonjourBrowser->_bonjourRecords.isEmpty())
    {
        _bonjourBrowser->cleanUp();
        _bonjourBrowser->browseForFoundServiceTypes();
    }else{
        qDebug() << "no devices found";
    }
}

void Explorer::startRecordResolve()
{
    _allRecords = _bonjourBrowser->_bonjourRecords;
    foreach(BonjourRecord b, _allRecords)
    {
        qDebug() << b.registeredType << b.replyDomain << b.serviceName;
    }

    getRecord();
}

/*
 * starts the resolving of records if there are any,
 * else emits finished()
 */

void Explorer::getRecord()
{
    if(!_allRecords.isEmpty())
    {
//        NOTE get parameter for excluding iphones->takes long, dns_sd maybe, never brings outcomes
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

/*
 * Removes first element from allRecords list so getRecord()
 * moves on with the next element
 */

void Explorer::prepareForNextRecord()
{
    _allRecords.removeFirst();
    getRecord();
}

/*
 * Deprecated:
 * Dispays elememts in groups with the registeredTape as header.
 * Now the saveResultsToDB() is used to save data to elasticsearch
 */

void Explorer::displayResults()
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
}

/*
 * Saves the results to an elasticsearch
 * via QNetworkAccessManager::post() and http
 * the results are sent. A timestamp, which is needed for elasticsearch
 * is added, too.
 */

void Explorer::saveResultsToDB()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    req.setUrl(QUrl(_db_url));
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addSecs(-60);
    QString date = dateTime.toString("yyyy-M-d");
    QString time = dateTime.toString("hh:mm:ss");
    QString formattedDateTime = date.append("T").append(time);

    QStringList keys = _results.keys();
    keys = keys.toSet().toList();
    foreach(QString key, keys)
    {
        QList<QMap<QString, QVariant> > membersOfOneClass = _results.values(key);
        QMap<QString, QVariant> m;
        foreach(m, membersOfOneClass)
        {
            QJsonObject *obj = new QJsonObject();
            obj->insert("post_date", formattedDateTime);
            obj->insert("registeredType", m.value("registeredType").toString());
            obj->insert("replyDomain", m.value("replyDomain").toString());
            obj->insert("ip", m.value("ip").toString());
            obj->insert("hostname", m.value("hostname").toString());
            obj->insert("lookupId", m.value("lookupId").toString());
            QJsonDocument doc(*obj);
            QString jsonString = doc.toJson(QJsonDocument::Compact);
            QByteArray data;
            data.append(jsonString);
            _rep = nam->post(req, data);
        }
    }
    connect(_rep, SIGNAL(readyRead()), this, SLOT(readAnswer()));
    qDebug() << "Found a total of " << _results.size() << "elements in " << keys.length() << "different categories.";
    //QCoreApplication::exit(0);
}

/*
 * Check if the results were saved successfully
 * and end the application
 */

void Explorer::readAnswer()
{
    QByteArray ba = _rep->readAll();
    if(ba.contains("\"created\":true")){
        qDebug() << "Successful: reply from server: " << ba;
        QCoreApplication::exit(0);
    }else if(ba.contains("\"created\":false")){
        qDebug() << "Unsuccessful: reply from server: " << ba;
        QCoreApplication::exit(-1);
    }
}

/*
 * Deprecated
 */

void Explorer::updateRecords(const QList<BonjourRecord> &list)
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
