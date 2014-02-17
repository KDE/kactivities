/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <boost/optional.hpp>

// Local
#include "utils/remove_if.h"
#include "utils_p.h"


namespace KActivities {
namespace Models {

struct ActivityModel::Private {
    DECLARE_RAII_MODEL_UPDATERS(ActivityModel)

    /**
     * Returns whether the the activity has a desired state.
     * If the state is 0, returns true
     */
    static inline bool matchingState(const QString &activityId,
                                     ActivityModel::State state)
    {
        // Are we filtering activities on their states?
        if (state) {
            // This is usually not advised (short-lived Info instance
            // but it comes with no cost since we have an already created
            // long-lived Controller instance
            Info activityInfo(activityId);
            if (activityInfo.state() != state)
                return false;
        }

        return true;
    }
    /**
     * Returns whether the the activity has a desired state.
     * If the state is 0, returns true
     */
    static inline bool matchingState(Info * activity,
                                     ActivityModel::State state)
    {
        // Are we filtering activities on their states?
        if (state) {
            if (activity->state() != state)
                return false;
        }

        return true;
    }

    /**
     * Searches for the activity.
     * Returns an option(index, iterator) for the found activity.
     */
    template <typename _Container>
    static inline
    boost::optional<
        std::pair<unsigned int, typename _Container::const_iterator>
    >
    activityPosition(const _Container &container, const QString &activityId)
    {
        using ActivityPosition =
                decltype(activityPosition(container, activityId));
        using ContainerElement =
                typename _Container::value_type;

        auto position = boost::find_if(container,
            [&] (const ContainerElement &activity) {
                return activity->id() == activityId;
            }
        );

        return (position != container.end()) ?
            ActivityPosition(
                std::make_pair(position - container.begin(), position)
            ) :
            ActivityPosition();
    }

    /**
     * Notifies the model that an activity was updated
     */
    template <typename _Model, typename _Container>
    static inline
    void emitActivityUpdated(_Model * model,
                             const _Container &container,
                             QObject * activityInfo, int role)
    {
        const auto activity = static_cast<Info*> (activityInfo);

        auto position = Private::activityPosition(container, activity->id());

        if (position) {
            emit model->dataChanged(
                model->index(position->first),
                model->index(position->first),
                QVector<int> {role}
            );
    }

    }
};

ActivityModel::ActivityModel(QObject *parent)
    : QAbstractListModel(parent), m_shownState(All)
{
    // Initializing role names for qml
    connect(&m_service, &Consumer::serviceStatusChanged,
            this,       &ActivityModel::setServiceStatus);

    connect(&m_service, SIGNAL(Consumer::activityAdded(QString)),
            this,       SLOT(ActivityModel::onActivityAdded(QString)));
    connect(&m_service, SIGNAL(Consumer::activityRemoved(QString)),
            this,       SLOT(ActivityModel::onActivityRemoved(QString)));

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

    m_registeredActivities.clear();
    m_shownActivities.clear();

    for (const QString &activity: activities) {
        onActivityAdded(activity, false);
    }
}

void ActivityModel::onActivityAdded(const QString &id, bool notifyClients)
{
    auto info = registerActivity(id);

    showActivity(info, notifyClients);
}

void ActivityModel::onActivityRemoved(const QString &id)
{
    hideActivity(id);
    unregisterActivity(id);
}

Info *ActivityModel::registerActivity(const QString &id)
{
    auto position = Private::activityPosition(m_registeredActivities, id);

    if (position) {
        return position->second->get();

    } else {
        auto activityInfo = new Info(id);

        connect(activityInfo, &Info::nameChanged,
                this,         &ActivityModel::onActivityNameChanged);
        connect(activityInfo, &Info::iconChanged,
                this,         &ActivityModel::onActivityIconChanged);
        connect(activityInfo, &Info::stateChanged,
                this,         &ActivityModel::onActivityStateChanged);

        m_registeredActivities.insert(InfoPtr(activityInfo));

        return activityInfo;
    }
}

void ActivityModel::unregisterActivity(const QString &id)
{
    auto position = Private::activityPosition(m_registeredActivities, id);

    if (position) {
        m_registeredActivities.erase(position->second);
    }
}

void ActivityModel::showActivity(Info *activityInfo, bool notifyClients)
{
    if (!Private::matchingState(activityInfo, m_shownState)) return;

    auto position = m_shownActivities.insert(activityInfo);

    if (notifyClients) {
        unsigned int index =
            (position.second ? position.first : m_shownActivities.end())
            - m_shownActivities.begin();

        Private::model_insert(this, QModelIndex(), index, index);
    }
}

void ActivityModel::hideActivity(const QString &id)
{
    auto position = Private::activityPosition(m_shownActivities, id);

    if (position) {
        Private::model_remove(this, QModelIndex(),
            position->first, position->first);
        m_shownActivities.erase(position->second);
    }
}

void ActivityModel::onActivityNameChanged(const QString &name)
{
    Q_UNUSED(name)

    Private::emitActivityUpdated(this, m_shownActivities, sender(),
                                 Qt::DisplayRole);
}

void ActivityModel::onActivityIconChanged(const QString &icon)
{
    Q_UNUSED(icon)

    Private::emitActivityUpdated(this, m_shownActivities, sender(),
                                 Qt::DecorationRole);
}

void ActivityModel::onActivityStateChanged(Info::State state)
{
    // TODO: Implement this properly
    // auto activity = static_cast<Info*> (sender());

    if (m_shownState == 0) {
        Private::emitActivityUpdated(this, m_shownActivities, sender(),
                                     ActivityState);

    } else {
        auto info = static_cast<Info*> (sender());

        if (state == m_shownState) {
            showActivity(info, true);
        } else {
            hideActivity(info->id());
        }
    }
}

void ActivityModel::setShownState(State state)
{
    m_shownState = state;

    replaceActivities(m_service.activities());

    emit shownStateChanged(state);
}

ActivityModel::State ActivityModel::shownState() const
{
    return m_shownState;
}

int ActivityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_shownActivities.size();
}

QVariant ActivityModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const auto &item = *(m_shownActivities.cbegin() + row);

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

QVariant ActivityModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
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
