/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ExtraActivitiesInterface.h"

#include <utils/d_ptr_implementation.h>

#include "utils/dbusfuture_p.h"
#include "features_interface.h"
#include "definitions.h"
#include "common/dbus/common.h"

#define ENABLE_QJSVALUE_CONTINUATION
#include "utils/continue_with.h"

class ExtraActivitiesInterface::Private {
public:
    Private(ExtraActivitiesInterface *q)
        : features(new KAMD_DBUS_CLASS_INTERFACE(Features, Features, q))
    {
    }

    ~Private()
    {
        delete features;
    }

    org::kde::ActivityManager::Features *features;
};

ExtraActivitiesInterface::ExtraActivitiesInterface(QObject *parent)
    : QObject(parent)
    , d(this)
{
}

ExtraActivitiesInterface::~ExtraActivitiesInterface()
{
}

void ExtraActivitiesInterface::setIsPrivate(const QString &id, bool isPrivate,
                                            QJSValue callback)
{
    auto result = d->features->SetValue(
        "org.kde.ActivityManager.Resources.Scoring/isOTR/" + id,
        QDBusVariant(isPrivate));

    auto *watcher = new QDBusPendingCallWatcher(result, this);

    QObject::connect(
            watcher, &QDBusPendingCallWatcher::finished,
            this, [callback] (QDBusPendingCallWatcher* watcher) mutable {
                callback.call();
                watcher->deleteLater();
            }
        );
}

void ExtraActivitiesInterface::getIsPrivate(const QString &id,
                                            QJSValue callback)
{
    auto result = d->features->GetValue(
        "org.kde.ActivityManager.Resources.Scoring/isOTR/" + id);

    auto *watcher = new QDBusPendingCallWatcher(result, this);

    QObject::connect(
            watcher, &QDBusPendingCallWatcher::finished,
            this, [callback,result] (QDBusPendingCallWatcher* watcher) mutable {
                QDBusPendingReply<QDBusVariant> reply = *watcher;
                callback.call({reply.value().variant().toBool()});
                watcher->deleteLater();
            }
        );
}

void ExtraActivitiesInterface::setShortcut(const QString &id,
                                           const QKeySequence &keySequence,
                                           QJSValue callback)
{
}

void ExtraActivitiesInterface::getShortcut(const QString &id,
                                           QJSValue callback)
{
}

