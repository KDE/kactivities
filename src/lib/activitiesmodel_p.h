/*
 *   Copyright (C) 2016 Ivan Čukić <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITIES_ACTIVITYMODEL_P_H
#define ACTIVITIES_ACTIVITYMODEL_P_H

#include "activitiesmodel.h"

#include "consumer.h"

#include "utils/qflatset.h"

namespace KActivities {

class ActivitiesModelPrivate : public QObject {
    Q_OBJECT
public:
    ActivitiesModelPrivate(ActivitiesModel *parent);

public Q_SLOTS:
    void onActivityNameChanged(const QString &name);
    void onActivityDescriptionChanged(const QString &description);
    void onActivityIconChanged(const QString &icon);
    void onActivityStateChanged(KActivities::Info::State state);

    void replaceActivities(const QStringList &activities);
    void onActivityAdded(const QString &id, bool notifyClients = true);
    void onActivityRemoved(const QString &id);
    void onCurrentActivityChanged(const QString &id);

    void setServiceStatus(KActivities::Consumer::ServiceStatus status);

public:
    KActivities::Consumer activities;
    QVector<Info::State> shownStates;

    typedef std::shared_ptr<Info> InfoPtr;

    struct InfoPtrComparator {
        bool operator() (const InfoPtr& left, const InfoPtr& right) const
        {
            const QString &leftName = left->name().toLower();
            const QString &rightName = right->name().toLower();

            return
                (leftName < rightName) ||
                (leftName == rightName && left->id() < right->id());
        }
    };

    QFlatSet<InfoPtr, InfoPtrComparator> knownActivities;
    QFlatSet<InfoPtr, InfoPtrComparator> shownActivities;

    InfoPtr registerActivity(const QString &id);
    void unregisterActivity(const QString &id);
    void showActivity(InfoPtr activityInfo, bool notifyClients);
    void hideActivity(const QString &id);
    void backgroundsUpdated(const QStringList &activities);

    InfoPtr findActivity(QObject *ptr) const;

    ActivitiesModel *const q;
};


} // namespace KActivities

#endif // ACTIVITIES_ACTIVITYMODEL_P_H

