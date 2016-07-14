/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KACTIVITIESINFO_P_H
#define KACTIVITIESINFO_P_H

#include "info.h"
#include <QObject>
#include <memory>

#include "activitiescache_p.h"

namespace KActivities {

class InfoPrivate {
public:
    InfoPrivate(Info *info, const QString &activity);

    void activityStateChanged(const QString &, int) const;

    void added(const QString &) const;
    void removed(const QString &) const;
    void started(const QString &) const;
    void stopped(const QString &) const;
    void infoChanged(const QString &) const;
    void nameChanged(const QString &, const QString &) const;
    void descriptionChanged(const QString &, const QString &) const;
    void iconChanged(const QString &, const QString &) const;
    void setServiceStatus(Consumer::ServiceStatus status) const;
    void setCurrentActivity(const QString &currentActivity);

    Info *const q;
    std::shared_ptr<ActivitiesCache> cache;
    bool isCurrent;

    const QString id;
};

} // namespace KActivities

#endif // ACTIVITIES_INFO_P_H
