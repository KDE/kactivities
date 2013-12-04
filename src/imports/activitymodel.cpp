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

// Self
#include "activitymodel.h"

// Qt
#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QModelIndex>

// KDE
#include <klocalizedstring.h>

// Boost
#include <boost/range/algorithm/find_if.hpp>

// Local
#include "utils/remove_if.h"
#include "utils_p.h"


namespace KActivities {
namespace Models {

struct ActivityModel::Private {
    DECLARE_RAII_MODEL_UPDATERS(ActivityModel)
};

ActivityModel::ActivityModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Initializing role names for qml
    connect(&m_consumer, &Consumer::serviceStatusChanged,
            this,        &ActivityModel::setServiceStatus);

    connect(&m_consumer, &Consumer::activityAdded,
            this,        &ActivityModel::addActivity);
    connect(&m_consumer, &Consumer::activityRemoved,
            this,        &ActivityModel::removeActivity);

    setServiceStatus(m_consumer.serviceStatus());
}

ActivityModel::~ActivityModel()
{
}

QHash<int, QByteArray> ActivityModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[Qt::DecorationRole] = "icon";
    roles[ActivityState] = "state";
    roles[ActivityId] = "id";
    return roles;
}


void ActivityModel::setServiceStatus(Consumer::ServiceStatus)
{
    replaceActivities(m_consumer.activities());
}

void ActivityModel::replaceActivities(const QStringList &activities)
{
    Private::model_reset m(this);

    m_activities.clear();

    for (const QString& activity: activities) {
        addActivitySilently(activity);
    }
}

void ActivityModel::addActivity(const QString &id)
{
    // Private::model_reset m(this);

    auto insertionPosition = addActivitySilently(id);

    Private::model_insert(this, QModelIndex(),
            insertionPosition, insertionPosition);
}

void ActivityModel::removeActivity(const QString &id)
{
    // Private::model_reset m(this);

    using namespace kamd::utils;

    auto position = boost::find_if(m_activities, [&] (std::unique_ptr<Info> &activity) {
        return activity->id() == id;
    });

    if (position != m_activities.end()) {
        auto insertionPosition = position - m_activities.begin();

        Private::model_remove(this, QModelIndex(),
            insertionPosition, insertionPosition);

        m_activities.erase(position);
    }

}

unsigned int ActivityModel::addActivitySilently(const QString &id)
{
    auto activity = new Info(id);

    connect(activity, &Info::nameChanged,
            this,     &ActivityModel::setActivityName);
    connect(activity, &Info::iconChanged,
            this,     &ActivityModel::setActivityIcon);
    connect(activity, &Info::stateChanged,
            this,     &ActivityModel::setActivityState);

    auto insertion = m_activities.insert(std::unique_ptr<Info>(activity));

    return (insertion.second ? insertion.first : m_activities.end())
           - m_activities.begin();
}

void ActivityModel::setActivityName(const QString &name)
{
    // const auto activity = static_cast<Info*> (sender());

    Private::model_reset m(this);

}

void ActivityModel::setActivityIcon(const QString &icon)
{
    // auto activity = static_cast<Info*> (sender());

    Private::model_reset m(this);

}

void ActivityModel::setActivityState(Info::State state)
{
    // auto activity = static_cast<Info*> (sender());

    Private::model_reset m(this);

}

int ActivityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_activities.size();
}

QVariant ActivityModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const auto &item = *(m_activities.cbegin() + row);

    switch (role) {
    case Qt::DisplayRole:
        return item->name();

    case Qt::DecorationRole:
        return QIcon::fromTheme(item->icon());

    case ActivityId:
        return item->id();

    case ActivityState:
        return item->state();

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
