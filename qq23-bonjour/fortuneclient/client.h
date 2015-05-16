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

#ifndef CLIENT_H
#define CLIENT_H

#include "bonjourrecord.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

#include <QTcpSocket>
#include <QHostInfo>
#include <QTimer>
class Client: public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client();

    int start();
    static const int _interval = 5000;

private slots:
    void updateRecords(const QList<BonjourRecord> &list);
    void displayError(QAbstractSocket::SocketError socketError);
    void listHosts(const QHostInfo &hostInfo);
    void checkResults();
    void handleDevice(QString device, BonjourRecord br);
    void handleDeviceClass(QString deviceClass);

signals:
    void foundDeviceClass(QString deviceClass);
    void foundDevice(QString device, BonjourRecord br);
    void hostFound();

private:
    QTimer *_timer;
    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;
    BonjourServiceBrowser *_bonjourBrowser;
    BonjourServiceResolver *_bonjourResolver;
    QList<QVariant> * allRecords;
    int _counter;
};

#endif
