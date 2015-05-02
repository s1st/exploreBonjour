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

#include <QGridLayout>
#include <QMessageBox>
#include <QtGui>
#include <QtNetwork>
#include <QLabel>

Client::Client()
    //:QObject(parent)
    :bonjourResolver(0)
{
    BonjourServiceBrowser *bonjourBrowser = new BonjourServiceBrowser(this);
    
    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));

    tcpSocket = new QTcpSocket(this);
    allRecords = new QList<QVariant>;
    requestNewFortune();
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

Client::~Client()
{

}

int Client::start()
{

    //    bonjourBrowser->browseForServiceType(QLatin1String("_trollfortune._tcp"));
    //    bonjourBrowser->browseForServiceType(QLatin1String("_services._dns-sd._udp"));
        bonjourBrowser->browseForServiceType(QLatin1String("_workstation._tcp"));

        //bonjourBrowser->browseForServiceType(QLatin1String("_apple-mobdev2._tcp"));
        //bonjourBrowser->browseForServiceType(QLatin1String("_googlecast._tcp"));
        //    bonjourBrowser->browseForServiceType(QLatin1String("_raop._tcp"));
        return 0;
}

void Client::requestNewFortune()
{
    blockSize = 0;
    tcpSocket->abort();
    if(allRecords->isEmpty())
    {
        return;
    }

    if (!bonjourResolver) {
        bonjourResolver = new BonjourServiceResolver(this);
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
                this, SLOT(connectToServer(const QHostInfo &, int)));
    }
//    QTreeWidgetItem *item = selectedItems.at(0);
    QVariant variant = allRecords->at(0);
    bonjourResolver->resolveBonjourRecord(variant.value<BonjourRecord>());
}

void Client::connectToServer(const QHostInfo &hostInfo, int port)
{
    const QList<QHostAddress> &addresses = hostInfo.addresses();
    if (!addresses.isEmpty())
        tcpSocket->connectToHost(addresses.first(), port);
}

void Client::readFortune()
{
    //TODO delete
//    QDataStream in(tcpSocket);
//    in.setVersion(QDataStream::Qt_4_0);

//    if (blockSize == 0) {
//        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
//            return;

//        in >> blockSize;
//    }

//    if (tcpSocket->bytesAvailable() < blockSize)
//        return;

//    QString nextFortune;
//    in >> nextFortune;

//    if (nextFortune == currentFortune) {
//        QTimer::singleShot(0, this, SLOT(requestNewFortune()));
//        return;
//    }

//    currentFortune = nextFortune;
//    statusLabel->setText(currentFortune);
//    getFortuneButton->setEnabled(true);
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

    //getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
//    getFortuneButton->setEnabled(treeWidget->invisibleRootItem()->childCount() != 0);
    //TODO delete
}

void Client::updateRecords(const QList<BonjourRecord> &list)
{
    QList<QVariant> vars;
    foreach (BonjourRecord record, list) {
        QVariant variant;
        variant.setValue(record);
        vars.append(variant);
        qDebug() << "###########";
        qDebug() << "type:" << record.registeredType;
        qDebug() << "domain:" << record.replyDomain;
        qDebug() << "serviceName:" << record.serviceName;
//        QTreeWidgetItem *processItem = new QTreeWidgetItem(treeWidget,
//                                                           QStringList() << record.serviceName);
        allRecords->append(variant);
//        processItem->setData(0, Qt::UserRole, variant);
    }
    
//    if (treeWidget->invisibleRootItem()->childCount() > 0) {
//        treeWidget->invisibleRootItem()->child(0)->setSelected(true);
//    }
    //QTreeWidgetItem *item = selectedItems.at(0);
    if (!bonjourResolver) {
        bonjourResolver = new BonjourServiceResolver(this);
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
                this, SLOT(connectToServer(const QHostInfo &, int)));
    }
    QVariant variant = vars.at(0);
    bonjourResolver->resolveBonjourRecord(variant.value<BonjourRecord>());
    enableGetFortuneButton();
}
