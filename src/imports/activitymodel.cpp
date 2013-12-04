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
#include <QFutureWatcher>
#include <QModelIndex>

// KDE
#include <klocalizedstring.h>

// Boost
#include <boost/range/algorithm/find_if.hpp>

// Local
#include "utils/remove_if.h"
// #include "utils/continuator.h"
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
    connect(&m_service, &Consumer::serviceStatusChanged,
            this,       &ActivityModel::setServiceStatus);

    connect(&m_service, &Consumer::activityAdded,
            this,       &ActivityModel::onActivityAdded);
    connect(&m_service, &Consumer::activityRemoved,
            this,       &ActivityModel::onActivityRemoved);

    setServiceStatus(m_service.serviceStatus());
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
    replaceActivities(m_service.activities());
}

void ActivityModel::replaceActivities(const QStringList &activities)
{
    Private::model_reset m(this);

    m_activities.clear();

    for (const QString& activity: activities) {
        addActivitySilently(activity);
    }
}

void ActivityModel::onActivityAdded(const QString &id)
{
    // Private::model_reset m(this);

    auto insertionPosition = addActivitySilently(id);

    Private::model_insert(this, QModelIndex(),
            insertionPosition, insertionPosition);
}

void ActivityModel::onActivityRemoved(const QString &id)
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
            this,     &ActivityModel::onActivityNameChanged);
    connect(activity, &Info::iconChanged,
            this,     &ActivityModel::onActivityIconChanged);
    connect(activity, &Info::stateChanged,
            this,     &ActivityModel::onActivityStateChanged);

    auto insertion = m_activities.insert(std::unique_ptr<Info>(activity));

    return (insertion.second ? insertion.first : m_activities.end())
           - m_activities.begin();
}

void ActivityModel::onActivityNameChanged(const QString &name)
{
    // TODO: Implement this properly
    // const auto activity = static_cast<Info*> (sender());

    Private::model_reset m(this);

}

void ActivityModel::onActivityIconChanged(const QString &icon)
{
    // TODO: Implement this properly
    // auto activity = static_cast<Info*> (sender());

    Private::model_reset m(this);

}

void ActivityModel::onActivityStateChanged(Info::State state)
{
    // TODO: Implement this properly
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

namespace {
    template <typename _ReturnType>
    void continue_with(const QFuture<_ReturnType> &future, QJSValue handler)
    {
        auto watcher = new QFutureWatcher<_ReturnType>();
        QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] () mutable {
                    handler.call(QJSValueList() << future.result());
                }
            );
        watcher->setFuture(future);
    }

    template <>
    void continue_with(const QFuture<void> &future, QJSValue handler)
    {
        auto watcher = new QFutureWatcher<void>();
        QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] () mutable {
                    handler.call(QJSValueList());
                }
            );
        watcher->setFuture(future);
    }
} // namespace

// QFuture<void> Controller::setActivityName(id, name)
void ActivityModel::setActivityName(const QString &id, const QString &name,
                                    const QJSValue &callback)
{
    continue_with(m_service.setActivityName(id, name), callback);
}

// QFuture<void> Controller::setActivityIcon(id, icon)
void ActivityModel::setActivityIcon(const QString &id, const QString &icon,
                                    const QJSValue &callback)
{
    continue_with(m_service.setActivityIcon(id, icon), callback);
}

// QFuture<bool> Controller::setCurrentActivity(id)
void ActivityModel::setCurrentActivity(const QString &id,
                                       const QJSValue &callback)
{
    continue_with(m_service.setCurrentActivity(id), callback);
}

// QFuture<QString> Controller::addActivity(name)
void ActivityModel::addActivity(const QString &name, const QJSValue &callback)
{
    continue_with(m_service.addActivity(name), callback);
}

// QFuture<void> Controller::removeActivity(id)
void ActivityModel::removeActivity(const QString &id, const QJSValue &callback)
{
    continue_with(m_service.removeActivity(id), callback);
}

// QFuture<void> Controller::stopActivity(id)
void ActivityModel::stopActivity(const QString &id, const QJSValue &callback)
{
    continue_with(m_service.stopActivity(id), callback);
}

// QFuture<void> Controller::startActivity(id)
void ActivityModel::startActivity(const QString &id, const QJSValue &callback)
{
    continue_with(m_service.startActivity(id), callback);
}


} // namespace Models
} // namespace KActivities

#include "activitymodel.moc"
