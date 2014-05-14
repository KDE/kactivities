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

#ifndef UTILS_P_H
#define UTILS_P_H

#include <QDBusPendingCallWatcher>

#include <QMutex>
#include <QDebug>

#include <KLocalizedString>

// Creates an async call to retrieve a value from the dbus service
// and initializes the call watcher
#define KAMD_RETRIEVE_REMOTE_VALUE(Variable, MethodToCall, Target)                      \
    Variable##Mutex.lock();                                                             \
    const QDBusPendingCall & Variable##Call = Manager::activities()->MethodToCall;      \
    Variable##CallWatcher = new QDBusPendingCallWatcher(Variable##Call, Target);        \
                                                                                        \
    QObject::connect(Variable##CallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)), \
            Target, SLOT(Variable##CallFinished(QDBusPendingCallWatcher*)))

// Defines a variable and handlers for a variable on a dbus service
// without a handler for when a call is finished
#define KAMD_REMOTE_VALUE_CUSTOM_HANDLER(Type, Name) \
    mutable Type Name;                               \
    QDBusPendingCallWatcher * Name##CallWatcher;     \
    QMutex Name##Mutex

// Defines a variable and handlers for a variable on a dbus service
#define KAMD_REMOTE_VALUE(Type, Name)                       \
    KAMD_REMOTE_VALUE_CUSTOM_HANDLER(Type, Name);           \
    void Name##CallFinished(QDBusPendingCallWatcher * call)

// macro defines a shorthand for validating and returning a d-bus result
// @param TYPE type of the result
// @param METHOD invocation of the d-bus method
// @param DEFAULT value to be used if the reply was not valid
#define KAMD_RETRIEVE_REMOTE_VALUE_SYNC(TYPE, OBJECT, METHOD, DEFAULT) \
    if (!Manager::isServicePresent()) return DEFAULT;              \
                                                                   \
    QDBusReply < TYPE > dbusReply = Manager::OBJECT()->METHOD;     \
    if (dbusReply.isValid()) {                                     \
        return dbusReply.value();                                  \
    } else {                                                       \
        qDebug() << "d-bus reply was invalid"                      \
                 << dbusReply.value()                              \
                 << dbusReply.error();                             \
        return DEFAULT;                                            \
    }

// Defines a handler for pre-fetching of the activity info
#define KAMD_RETRIEVE_REMOTE_VALUE_HANDLER(ReturnType, Namespace, Variable, DefaultValue)   \
    void Namespace::Variable##CallFinished(QDBusPendingCallWatcher * call)       \
    {                                                                            \
        QDBusPendingReply <ReturnType> reply = * call;                           \
                                                                                 \
        Variable = reply.isError()                                               \
            ? DefaultValue                                                       \
            : reply.argumentAt<0>();                                             \
                                                                                 \
        Variable##CallWatcher = 0;                                               \
        Variable##Mutex.unlock();                                                \
        call->deleteLater();                                                     \
    }

// Implements a value getter
#define KAMD_REMOTE_VALUE_GETTER(ReturnType, Namespace, Variable, Default) \
    ReturnType Namespace::Variable() const                            \
    {                                                                 \
        if (!Manager::isServicePresent()) return Default;             \
        waitForCallFinished(d->Variable##CallWatcher, &d->Variable##Mutex); \
        return d->Variable;                                           \
    }

static inline void waitForCallFinished(QDBusPendingCallWatcher * watcher, QMutex * mutex)
{
    if (watcher) {
        watcher->waitForFinished();

        mutex->lock();
        mutex->unlock();
    }
}

#endif // UTILS_P_H

