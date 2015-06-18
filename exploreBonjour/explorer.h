/****************************************************************************
** With this bonjour explorer you can find bonjour/zeroconf devices
** in a network.
**
** This programm is inspired by
** and bonjourservicebrowser and bonjourserviceresolver are based on
** qt quarterly 23:
**
** http://doc.qt.io/archives/qq/qq23-bonjour.html
**
** Old header comment:
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
#include <QNetworkReply>

/*
 * Url for database running on port 9200
 * with the index bonjour and the type
 * b_entry:
 * "http://localhost:9200/bonjour/b_entry"
 */

static const QString _db_url = "http://localhost:9200/bonjour/b_entry";

class Explorer: public QObject
{
    Q_OBJECT

public:
    Explorer();
    ~Explorer();

    int start();
    static const int _exploreInterval = 2000;
    static const int _browseInterval = 300;

private slots:
    void updateRecords(const QList<BonjourRecord> &list);
    void saveHostInformation(const QHostInfo &hostInfo);
    void checkResults();
    void startRecordResolve();
    void getRecord();
    void prepareForNextRecord();
    void displayResults();
    void saveResultsToDB();
    void readAnswer();

signals:
    void hostInfoSaved();
    void finished();

private:
    QNetworkReply *_rep;
    BonjourServiceBrowser *_bonjourBrowser;
    BonjourServiceResolver *_bonjourResolver;
    QList<BonjourRecord> _allRecords;
    QMultiMap<QString, QMap<QString, QVariant> > _results;
};

#endif
