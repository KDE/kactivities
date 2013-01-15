/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KACTIVITIESINFO_P_H
#define KACTIVITIESINFO_P_H

#include "info.h"
#include <QObject>

#include "utils_p.h"

namespace KActivities {

class InfoPrivateCommon: public QObject {
    Q_OBJECT
public:
    static InfoPrivateCommon * self();

    InfoPrivateCommon();
    virtual ~InfoPrivateCommon();

private:
    static InfoPrivateCommon * s_instance;
};

class InfoPrivate {
public:
    InfoPrivate(Info * info, const QString & activity);

    void activityStateChanged(const QString &, int);

    void added(const QString &) const;
    void removed(const QString &) const;
    void started(const QString &) const;
    void stopped(const QString &) const;
    void infoChanged(const QString &) const;
    void nameChanged(const QString &, const QString &) const;
    void iconChanged(const QString &, const QString &) const;
    void setServicePresent(bool present);

    void initializeCachedData();

    Info *q;
    Info::State state;

    KAMD_REMOTE_VALUE(QString, name);
    KAMD_REMOTE_VALUE(QString, icon);

    const QString id;
};

} // namespace KActivities

#endif // ACTIVITIES_INFO_P_H
