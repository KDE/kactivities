/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITIES_CACHE_P_H
#define ACTIVITIES_CACHE_P_H

#include <memory>

#include <QObject>

#include <common/dbus/org.kde.ActivityManager.Activities.h>
#include <utils/ptr_to.h>

#include "activities_interface.h"
#include "consumer.h"

namespace KActivities {

class ActivitiesCache : public QObject {
    Q_OBJECT

public:
    static std::shared_ptr<ActivitiesCache> self();

    ~ActivitiesCache();

Q_SIGNALS:
    void activityAdded(const QString &id);
    void activityChanged(const QString &id);
    void activityRemoved(const QString &id);

    void activityStateChanged(const QString &id, int state);
    void activityNameChanged(const QString &id, const QString &name);
    void activityDescriptionChanged(const QString &id, const QString &description);
    void activityIconChanged(const QString &id, const QString &icon);

    void currentActivityChanged(const QString &id);
    void serviceStatusChanged(Consumer::ServiceStatus status);
    void activityListChanged();

private Q_SLOTS:
    void updateAllActivities();
    void loadOfflineDefaults();

    void updateActivity(const QString &id);
    void updateActivityState(const QString &id, int state);
    void removeActivity(const QString &id);

    void setActivityInfoFromReply(QDBusPendingCallWatcher *watcher);
    void setAllActivitiesFromReply(QDBusPendingCallWatcher *watcher);
    void setCurrentActivityFromReply(QDBusPendingCallWatcher *watcher);

    void setActivityName(const QString &id, const QString &name);
    void setActivityDescription(const QString &id, const QString &description);
    void setActivityIcon(const QString &id, const QString &icon);

    void setActivityInfo(const ActivityInfo &info);
    void setAllActivities(const ActivityInfoList &activities);
    void setCurrentActivity(const QString &activity);

    void setServiceStatus(bool status);

public:
    template <typename _Result, typename _Functor>
    void passInfoFromReply(QDBusPendingCallWatcher *watcher, _Functor f);

    template <int Policy = kamd::utils::Const>
    inline typename kamd::utils::ptr_to<ActivityInfo, Policy>::type
    find(const ActivityInfo &info)
    {
        auto where
            = std::lower_bound(m_activities.begin(), m_activities.end(), info);

        if (where != m_activities.end() && where->id == info.id) {
            return &(*where);
        }

        return Q_NULLPTR;
    }

    template <int Policy = kamd::utils::Const>
    inline typename kamd::utils::ptr_to<ActivityInfo, Policy>::type
    find(const QString &id)
    {
        return find<Policy>(ActivityInfo(id));
    }

    ActivitiesCache();

    QList<ActivityInfo> m_activities;
    QString m_currentActivity;
    Consumer::ServiceStatus m_status;
};

} // namespace KActivities


#endif /* ACTIVITIES_CACHE_P_H */

