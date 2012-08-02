/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSMSERVER_P_H
#define KSMSERVER_P_H

#include "KSMServer.h"

#include <QPair>

class QDBusServiceWatcher;
class QDBusInterface;
class QDBusPendingCallWatcher;

class KSMServer::Private: public QObject {
    Q_OBJECT

public:
    Private(KSMServer * parent);

    void processLater(const QString & activity, bool start);

private Q_SLOTS:
    void serviceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner);

    void process();
    void makeRunning(bool value);

    void startCallFinished(QDBusPendingCallWatcher * watcher);
    void stopCallFinished(QDBusPendingCallWatcher * watcher);

    void subSessionOpened();
    void subSessionClosed();
    void subSessionCloseCanceled();
    void subSessionSendEvent(int event);

private:
    QDBusServiceWatcher * serviceWatcher;
    QDBusInterface * kwin;
    QDBusInterface * ksmserver;

    bool processing;
    QString processingActivity;
    QList < QPair < QString, bool > > queue;

    KSMServer * const q;
};

#endif // KSMSERVER_P_H
