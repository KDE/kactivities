/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#include <QDBusConnection>
#include <QDBusInterface>

// clang-format off

#define KAMD_DBUS_SERVICE                                                      \
    QStringLiteral("org.kde.ActivityManager")

#define KAMD_DBUS_OBJECT_PATH(A)                                               \
    (sizeof(A) > 2 ? QLatin1String("/ActivityManager/" A)                      \
                   : QLatin1String("/ActivityManager"))

#define KAMD_DBUS_OBJECT(A)                                                    \
    QLatin1String("org.kde.ActivityManager." A)

#define KAMD_DBUS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                       \
    QDBusInterface(KAMD_DBUS_SERVICE,                                          \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   PARENT)

#define KAMD_DBUS_DECL_INTERFACE(VAR, OBJECT_PATH, OBJECT)                     \
    QDBusInterface VAR(KAMD_DBUS_SERVICE,                                      \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   nullptr)

#define KAMD_DBUS_CLASS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                 \
    org::kde::ActivityManager::OBJECT(                                         \
                KAMD_DBUS_SERVICE,                                             \
                KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                            \
                QDBusConnection::sessionBus(),                                 \
                PARENT)

#endif // DBUS_COMMON_H

