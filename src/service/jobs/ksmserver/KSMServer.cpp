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

#include "KSMServer.h"
#include "KSMServer_p.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QDebug>

#include <kdbusconnectionpool.h>

#include <utils/d_ptr_implementation.h>
#include <utils/val.h>

#define KWIN_SERVICE "org.kde.kwin"
#define KSMSERVER_SERVICE "org.kde.ksmserver"

KSMServer::Private::Private(KSMServer * parent)
  : serviceWatcher(nullptr),
    kwin(nullptr),
    ksmserver(nullptr),
    processing(false),
    q(parent)
{
    serviceWatcher = new QDBusServiceWatcher(this);

    serviceWatcher->setConnection(KDBusConnectionPool::threadConnection());
    serviceWatcher->addWatchedService(KWIN_SERVICE);
    serviceWatcher->addWatchedService(KSMSERVER_SERVICE);

    connect(serviceWatcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)),
            this, SLOT(serviceOwnerChanged(QString, QString, QString)));

    serviceOwnerChanged(KWIN_SERVICE,      QString(), QString());
    serviceOwnerChanged(KSMSERVER_SERVICE, QString(), QString());
}

template < typename Func >
static void initializeInterface(QDBusInterface * & service,
        const QString & servicePath, const QString & path, const QString & object, Func init)
{
    // Delete the old object, just in case
    delete service;

    // Creating the new dbus interface
    service = new QDBusInterface(servicePath, path, object);

    qDebug() << path << "is valid?" << service->isValid();

    // If the service is valid, initialize it
    // otherwise delete the object
    if (service->isValid()) {
        init(service);

    } else {
        delete service;
        service = nullptr;

    }

}

void KSMServer::Private::serviceOwnerChanged(const QString & service,
        const QString & oldOwner, const QString & newOwner)
{
    Q_UNUSED(oldOwner);
    Q_UNUSED(newOwner);

    if (service == KSMSERVER_SERVICE) {

        initializeInterface(
            ksmserver,
            KSMSERVER_SERVICE, "/KSMServer", "org.kde.KSMServerInterface",
            [this] (QObject * service) {
                service->setParent(this);
                connect(service, SIGNAL(subSessionOpened()),
                    this, SLOT(subSessionOpened()));
                connect(service, SIGNAL(subSessionClosed()),
                    this, SLOT(subSessionClosed()));
                connect(service, SIGNAL(subSessionCloseCanceled()),
                    this, SLOT(subSessionCloseCanceled()));
            }
        );

    } else if (service == KWIN_SERVICE) {

        initializeInterface(
            kwin,
            KWIN_SERVICE, "/KWin", "org.kde.KWin",
            [this] (QObject * service) {
                service->setParent(this);
            }
        );

    }
}

KSMServer::KSMServer(QObject * parent)
  : QObject(parent), d(this)
{
}

KSMServer::~KSMServer()
{
}

void KSMServer::startActivitySession(const QString & activity)
{
    d->processLater(activity, true);
}

void KSMServer::stopActivitySession(const QString & activity)
{
    d->processLater(activity, false);
}

void KSMServer::Private::processLater(const QString & activity, bool start)
{
    qDebug() << "Scheduling" << activity << "to be" << (start ? "started" : "stopped");

    foreach (val & item, queue) {
        if (item.first == activity) {
            return;
        }
    }

    queue << qMakePair(activity, start);

    if (!processing) {
        processing = true;
        QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
    }
}

void KSMServer::Private::process()
{
    // If the queue is empty, we have nothing more to do
    if (queue.isEmpty()) {
        processing = false;
        return;
    }


    val item = queue.takeFirst();
    processingActivity = item.first;

    qDebug() << "Processing" << item;

    makeRunning(item.second);

    // Calling process again for the rest of the list
    QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
}

void KSMServer::Private::makeRunning(bool value)
{
    if (!kwin) {
        qDebug() << "There is no KWin";
        subSessionSendEvent(value ? KSMServer::Started : KSMServer::Stopped);
        return;
    }

    val call = kwin->asyncCall(
            value ? QLatin1String("startActivity") : QLatin1String("stopActivity"),
            processingActivity);

    val watcher = new QDBusPendingCallWatcher(call, this);

    QObject::connect(
        watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        this,
            value
              ? SLOT(startCallFinished(QDBusPendingCallWatcher*))
              : SLOT(stopCallFinished(QDBusPendingCallWatcher*))
    );
}

void KSMServer::Private::startCallFinished(QDBusPendingCallWatcher * call)
{
     QDBusPendingReply < bool > reply = * call;

     if (reply.isError()) {
         qDebug() << "Session starting call failed, but we are returning success";
         emit q->activitySessionStateChanged(processingActivity, KSMServer::Started);

     } else {
         // If we got false, it means something is going on with ksmserver
         // and it didn't start our activity
         val retval = reply.argumentAt<0>();

         qDebug() << "Did we start the activity successfully:" << retval;
         if (!retval) {
            subSessionSendEvent(KSMServer::Stopped);
         }
     }

     call->deleteLater();
}

void KSMServer::Private::stopCallFinished(QDBusPendingCallWatcher * call)
{
    QDBusPendingReply < bool > reply = * call;

    if (reply.isError()) {
        qDebug() << "Session stopping call failed, but we are returning success";
        emit q->activitySessionStateChanged(processingActivity, KSMServer::Stopped);

    } else {
        // If we got false, it means something is going on with ksmserver
        // and it didn't stop our activity
        val retval = reply.argumentAt<0>();

        qDebug() << "Did we stop the activity successfully:" << retval;
        if (!retval) {
            subSessionSendEvent(KSMServer::FailedToStop);
        }
    }

    call->deleteLater();
}

void KSMServer::Private::subSessionSendEvent(int event)
{
    if (processingActivity.isEmpty()) return;

    emit q->activitySessionStateChanged(processingActivity, event);

    processingActivity.clear();
}

void KSMServer::Private::subSessionOpened()
{
    subSessionSendEvent(KSMServer::Started);
}

void KSMServer::Private::subSessionClosed()
{
    subSessionSendEvent(KSMServer::Stopped);
}

void KSMServer::Private::subSessionCloseCanceled()
{
    subSessionSendEvent(KSMServer::FailedToStop);
}

