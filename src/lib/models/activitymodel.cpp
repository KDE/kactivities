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

#include "activitymodel.h"

#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QDebug>

#include <KIcon>
#include <KLocalizedString>

#include <common/dbus/org.kde.ActivityManager.Activities.h>

// from libkactivities (core)
#include "../core/manager_p.h"
#include "../core/utils_p.h"

#include "utils_p.h"

namespace KActivities {
namespace Models {

static bool activityInfoLessThen(const ActivityInfo & left, const ActivityInfo & right)
{
    return left.name.toLower() < right.name.toLower();
}

class ActivityModel::Private {
public:
    DECLARE_RAII_MODEL_UPDATERS(ActivityModel)

    Private(ActivityModel * parent)
        : q(parent), valid(false)
    {
        qDebug() << "Manager isServicePresent" << Manager::isServicePresent();
        if (Manager::isServicePresent())
            fetchActivityList();

        connect(Manager::self(), SIGNAL(servicePresenceChanged(bool)),
                q, SLOT(servicePresenceChanged(bool)));

        connect(Manager::activities(), SIGNAL(ActivityAdded(QString)),
                q, SLOT(activityAdded(QString)));
        connect(Manager::activities(), SIGNAL(ActivityRemoved(QString)),
                q, SLOT(activityRemoved(QString)));
        connect(Manager::activities(), SIGNAL(ActivityNameChanged(QString, QString)),
                q, SLOT(activityNameChanged(QString, QString)));
        connect(Manager::activities(), SIGNAL(ActivityIconChanged(QString, QString)),
                q, SLOT(activityIconChanged(QString, QString)));
        connect(Manager::activities(), SIGNAL(ActivityStateChanged(QString, int)),
                q, SLOT(activityStateChanged(QString, int)));
    }

    void fetchActivityList();
    void fetchActivityInfo(const QString & activity);

    void listActivitiesCallFinished(QDBusPendingCallWatcher * watcher);
    void activityInfoCallFinished(QDBusPendingCallWatcher * watcher);
    QMutex listActivitiesMutex;
    QMutex activityInfoMutex;

    void activityNameChanged(const QString & activity, const QString & name);
    void activityIconChanged(const QString & activity, const QString & icon);
    void activityStateChanged(const QString & activity, int state);

    void activityAdded(const QString & activity);
    void activityRemoved(const QString & activity);

    void servicePresenceChanged(bool present);

    QList < ActivityInfo > activities;
    QHash < QString, int > activityIndex;

    ActivityModel * const q;

    QDBusPendingCallWatcher * listActivitiesCallWatcher;

    bool valid : 1;
};

void ActivityModel::Private::servicePresenceChanged(bool present)
{
    Q_UNUSED(present)

    model_reset m(q);

    valid = false;

    if (valid) fetchActivityList();
}

void ActivityModel::Private::fetchActivityList()
{
    qDebug() << "getting the list of activities";
    KAMD_RETRIEVE_REMOTE_VALUE(listActivities, ListActivitiesWithInformation(), q);
}

void ActivityModel::Private::fetchActivityInfo(const QString & activity)
{
    QDBusPendingCallWatcher * activityInfoCallWatcher;
    qDebug() << "getting info for " << activity;
    KAMD_RETRIEVE_REMOTE_VALUE(activityInfo, ActivityInformation(activity), q);
}

void ActivityModel::Private::listActivitiesCallFinished(QDBusPendingCallWatcher * watcher)
{
    qDebug() << "got the activities";
    model_reset m(q);

    QDBusPendingReply <ActivityInfoList> reply = * watcher;

    if (reply.isError()) {
        valid = false;
        qDebug() << "we got some kind of error" << reply.error();
        return;
    }

    activities = reply.argumentAt<0>();

    qSort(activities.begin(), activities.end(), activityInfoLessThen);

    for (int i = 0; i < activities.size(); i++) {
        activityIndex[activities[i].id] = i;
    }

    valid = true;

    qDebug() << activities.size();

    watcher->deleteLater();
}

void ActivityModel::Private::activityInfoCallFinished(QDBusPendingCallWatcher * watcher)
{
    qDebug() << "got the activities";

    QDBusPendingReply <ActivityInfo> reply = * watcher;

    if (reply.isError()) {
        valid = false;
        qDebug() << "we got some kind of error" << reply.error();
        return;
    }

    ActivityInfo info = reply.argumentAt<0>();

    QList < ActivityInfo > ::iterator insertAt
         = qLowerBound(activities.begin(), activities.end(), info, activityInfoLessThen);

    insertAt = activities.insert(insertAt, info);

    int index = insertAt - activities.begin();
    model_insert m(q, QModelIndex(), index, index);

    QMutableHashIterator < QString, int > i(activityIndex);
    while (i.hasNext()) {
        i.next();

        const int value = i.value();
        if (value >= index)
            i.setValue(value + 1);
    }

    activityIndex[info.id] = index;

    watcher->deleteLater();
}

#define PROPERTY_CHANGED_HANDLER(Name, StrName, Type)                               \
    void ActivityModel::Private::Name##Changed(const QString & activity, Type StrName) \
    {                                                                               \
        if (!activityIndex.contains(activity)) return;                              \
                                                                                    \
        const int index = activityIndex[activity];                                  \
        activities[index].StrName = StrName;                                        \
                                                                                    \
        QModelIndex i = q->index(index);                                            \
        q->dataChanged(i, i);                                                       \
    }

PROPERTY_CHANGED_HANDLER(activityName,  name,  const QString &)
PROPERTY_CHANGED_HANDLER(activityIcon,  icon,  const QString &)
PROPERTY_CHANGED_HANDLER(activityState, state, int)

#undef PROPERTY_CHANGED_HANDLER

void ActivityModel::Private::activityAdded(const QString & activity)
{
    fetchActivityInfo(activity);
}

void ActivityModel::Private::activityRemoved(const QString & activity)
{
    if (!activityIndex.contains(activity)) return;

    int index = activityIndex[activity];
    model_remove m(q, QModelIndex(), index, index);

    activities.removeAt(index);
    activityIndex.remove(activity);

    QMutableHashIterator < QString, int > i(activityIndex);
    while (i.hasNext()) {
        i.next();

        const int value = i.value();
        if (value > index)
            i.setValue(value - 1);
    }
}


ActivityModel::ActivityModel(QObject * parent)
    : QAbstractListModel(parent), d(new Private(this))
{
    qDebug() << "################";
    d->valid = false;

    QHash<int, QByteArray> roles;

    roles[Qt::DisplayRole]    = "name";
    roles[Qt::DecorationRole] = "icon";
    roles[ActivityState]      = "state";
    roles[ActivityId]         = "id";

    setRoleNames(roles);
}

ActivityModel::~ActivityModel()
{
    delete d;
}

int ActivityModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    if (!d->valid) return 0;

    return d->activities.size();
}

QVariant ActivityModel::data(const QModelIndex & index, int role) const
{
    if (!d->valid) return QVariant();

    const int row = index.row();

    switch (role) {
        case Qt::DisplayRole:
            return d->activities[row].name;

        case Qt::DecorationRole:
            return KIcon(d->activities[row].icon);

        case ActivityId:
            return d->activities[row].id;

        case ActivityState:
            return d->activities[row].state;

        default:
            return QVariant();
    }
}

QVariant ActivityModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (section == 0 && role == Qt::DisplayRole) {
        return i18nc("Header title for activity data model", "Activity");
    }

    return QVariant();
}

} // namespace Models
} // namespace KActivities

#include "activitymodel.moc"
