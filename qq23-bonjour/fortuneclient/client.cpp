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

#include <QtGui>
#include <QtNetwork>

#include "client.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

Client::Client(QWidget *parent)
    : QDialog(parent), bonjourResolver(0)
{
    BonjourServiceBrowser *bonjourBrowser = new BonjourServiceBrowser(this);
    
    statusLabel = new QLabel("This examples requires that you run the "
                                "Fortune Server example as well.");

    treeWidget = new QTreeWidget(this);
    treeWidget->setHeaderLabels(QStringList() << tr("Available Fortune Servers"));
    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
    getFortuneButton = new QPushButton(tr("Get Fortune"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(treeWidget, 0, 0, 2, 2);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Fortune Client"));
    treeWidget->setFocus();
    bonjourBrowser->browseForServiceType(QLatin1String("_trollfortune._tcp"));
}

void Client::requestNewFortune()
{
    getFortuneButton->setEnabled(false);
    blockSize = 0;
    tcpSocket->abort();
    QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
    if (selectedItems.isEmpty())
        return;

    if (!bonjourResolver) {
        bonjourResolver = new BonjourServiceResolver(this);
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
                this, SLOT(connectToServer(const QHostInfo &, int)));
    }
    QTreeWidgetItem *item = selectedItems.at(0);
    QVariant variant = item->data(0, Qt::UserRole);
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
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;

        in >> blockSize;
    }

    if (tcpSocket->bytesAvailable() < blockSize)
        return;

    QString nextFortune;
    in >> nextFortune;

    if (nextFortune == currentFortune) {
        QTimer::singleShot(0, this, SLOT(requestNewFortune()));
        return;
    }

    currentFortune = nextFortune;
    statusLabel->setText(currentFortune);
    getFortuneButton->setEnabled(true);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
    getFortuneButton->setEnabled(treeWidget->invisibleRootItem()->childCount() != 0);
}

void Client::updateRecords(const QList<BonjourRecord> &list)
{
    treeWidget->clear();
    foreach (BonjourRecord record, list) {
        QVariant variant;
        variant.setValue(record);
        QTreeWidgetItem *processItem = new QTreeWidgetItem(treeWidget,
                                                           QStringList() << record.serviceName);
        processItem->setData(0, Qt::UserRole, variant);
    }
    
    if (treeWidget->invisibleRootItem()->childCount() > 0) {
        treeWidget->invisibleRootItem()->child(0)->setSelected(true);
    }
    enableGetFortuneButton();
}
