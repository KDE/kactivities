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
#include <kconfig.h>
#include <kconfiggroup.h>

// Boost
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/binary_search.hpp>
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
    template <typename T>
    static inline bool matchingState(const QString &activityId,
                                     T states)
    {
        // Are we filtering activities on their states?
        if (!states.empty()) {
            // This is usually not advised (short-lived Info instance
            // but it comes with no cost since we have an already created
            // long-lived Controller instance
            Info activityInfo(activityId);
            if (!boost::binary_search(states, activityInfo.state()))
                return false;
        }

        return true;
    }
    /**
     * Returns whether the the activity has a desired state.
     * If the state is 0, returns true
     */
    template <typename T>
    static inline bool matchingState(Info * activity,
                                     T states)
    {
        // Are we filtering activities on their states?
        if (!states.empty()) {
            if (!boost::binary_search(states, activity->state()))
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

        // qDebug() << "Activity updated: " << activity->id()
        //          << "name: " << activity->name()
        //          ;

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

static
KConfigGroup _plasmaConfigContainments() {
    static KConfig config("plasma-org.kde.desktop-appletsrc");
    return config.group("Containments");
}


ActivityModel::ActivityModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Initializing role names for qml
    connect(&m_service, &Consumer::serviceStatusChanged,
            this,       &ActivityModel::setServiceStatus);

    connect(&m_service, SIGNAL(activityAdded(QString)),
            this,       SLOT(onActivityAdded(QString)));
    connect(&m_service, SIGNAL(activityRemoved(QString)),
            this,       SLOT(onActivityRemoved(QString)));

    setServiceStatus(m_service.serviceStatus());
}

ActivityModel::~ActivityModel()
{
}

QHash<int, QByteArray> ActivityModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole]    = "name";
    roles[Qt::DecorationRole] = "icon";
    roles[ActivityState]      = "state";
    roles[ActivityId]         = "id";
    roles[ActivityBackground] = "background";
    roles[ActivityCurrent]    = "current";
    return roles;
}


void ActivityModel::setServiceStatus(Consumer::ServiceStatus)
{
    replaceActivities(m_service.activities());
}

void ActivityModel::replaceActivities(const QStringList &activities)
{
    // qDebug() << m_shownStatesString << "New list of activities: " << activities;
    // qDebug() << m_shownStatesString << " -- RESET MODEL -- ";

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

    // qDebug() << m_shownStatesString << "Added a new activity:" << info->id() << " " << info->name();

    showActivity(info, notifyClients);
}

void ActivityModel::onActivityRemoved(const QString &id)
{
    // qDebug() << m_shownStatesString << "Removed an activity:" << id;

    hideActivity(id);
    unregisterActivity(id);
}

Info *ActivityModel::registerActivity(const QString &id)
{
    auto position = Private::activityPosition(m_registeredActivities, id);

    // qDebug() << m_shownStatesString << "Registering activity: " << id
    //          << " new? not " << (bool)position;

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
    // qDebug() << m_shownStatesString << "Deregistering activity: " << id;

    auto position = Private::activityPosition(m_registeredActivities, id);

    if (position) {
        m_registeredActivities.erase(position->second);
    }
}

void ActivityModel::showActivity(Info *activityInfo, bool notifyClients)
{
    // Should it really be shown?
    if (!Private::matchingState(activityInfo, m_shownStates)) return;

    // Is it already shown?
    if (boost::binary_search(m_shownActivities, activityInfo, InfoPtrComparator())) return;

    // qDebug() << m_shownStatesString << "Setting activity visibility to true: "
    //     << activityInfo->id() << activityInfo->name();

    auto position = m_shownActivities.insert(activityInfo);

    if (notifyClients) {
        unsigned int index =
            (position.second ? position.first : m_shownActivities.end())
            - m_shownActivities.begin();

        // qDebug() << m_shownStatesString << " -- MODEL INSERT -- " << index;
        Private::model_insert(this, QModelIndex(), index, index);
    }
}

void ActivityModel::hideActivity(const QString &id)
{
    auto position = Private::activityPosition(m_shownActivities, id);

    // qDebug() << m_shownStatesString << "Setting activity visibility to false: " << id;

    if (position) {
        // qDebug() << m_shownStatesString << " -- MODEL REMOVE -- " << position->first;
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

    if (m_shownStates.empty()) {
        Private::emitActivityUpdated(this, m_shownActivities, sender(),
                                     ActivityState);

    } else {
        auto info = static_cast<Info*> (sender());

        // qDebug() << m_shownStatesString << "Activity state has changed: "
        //          << info->id() << " " << info->name()
        //          << " to: " << state;

        if (boost::binary_search(m_shownStates, state)) {
            showActivity(info, true);
        } else {
            hideActivity(info->id());
        }
    }
}

void ActivityModel::setShownStates(const QString &states)
{
    m_shownStates.clear();
    m_shownStatesString = states;

    for (const auto &state: states.split(',')) {
        if (state == QStringLiteral("Running")) {
            m_shownStates.insert(Running);

        } else if (state == QStringLiteral("Starting")) {
            m_shownStates.insert(Starting);

        } else if (state == QStringLiteral("Stopped")) {
            m_shownStates.insert(Stopped);

        } else if (state == QStringLiteral("Stopping")) {
            m_shownStates.insert(Stopping);

        }
    }

    replaceActivities(m_service.activities());

    emit shownStatesChanged(states);
}

QString ActivityModel::shownStates() const
{
    return m_shownStatesString;
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

    case ActivityCurrent:
        return m_service.currentActivity() == item->id();

    case ActivityBackground:
        {
            qDebug() << "GAGAGAGAGAG:" << _plasmaConfigContainments().groupList();
            qDebug() << "Searching for activity: " << item->name() << " " << item->id();

            for (const auto &group: _plasmaConfigContainments().groupList()) {
                auto containmentGroup = _plasmaConfigContainments().group(group);

                qDebug() << "Found: " << containmentGroup.readEntry("activityId");

                if (containmentGroup.readEntry("activityId", QString()) == item->id()) {
                    auto wallpaper = containmentGroup
                        .group("Wallpaper")
                        .group("General")
                        .readEntry("Image", QString());

                    if (!wallpaper.isEmpty())
                        return wallpaper;
                }
            }

            return "Gaga!";
        }

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
