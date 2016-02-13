/*
 *   Copyright (C) 2012 - 2016 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include "activitiesmodel.h"
#include "activitiesmodel_p.h"

// Qt
#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QFutureWatcher>
#include <QModelIndex>

// Boost
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/binary_search.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/optional.hpp>

// Local
#include "utils/remove_if.h"
#include "utils/model_updaters.h"

namespace KActivities {

namespace Private {
    // DECLARE_RAII_MODEL_UPDATERS(ActivitiesModel)

    /**
     * Returns whether the the activity has a desired state.
     * If the state is 0, returns true
     */
    template <typename T>
    inline bool matchingState(ActivitiesModelPrivate::InfoPtr activity,
                                     T states)
    {
        // Are we filtering activities on their states?
        if (!states.empty()
            && !boost::binary_search(states, activity->state())) {
            return false;
        }

        return true;
    }

    /**
     * Searches for the activity.
     * Returns an option(index, iterator) for the found activity.
     */
    template <typename _Container>
    inline
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
    inline
    void emitActivityUpdated(_Model *model,
                             const _Container &container,
                             const QString &activity, int role)
    {
        auto position = Private::activityPosition(container, activity);

        if (position) {
            emit model->q->dataChanged(
                model->q->index(position->first),
                model->q->index(position->first),
                role == Qt::DecorationRole ?
                    QVector<int> {role, ActivitiesModel::ActivityIcon} :
                    QVector<int> {role}
            );
        }
    }

    /**
     * Notifies the model that an activity was updated
     */
    template <typename _Model, typename _Container>
    inline
    void emitActivityUpdated(_Model *model,
                             const _Container &container,
                             QObject *activityInfo, int role)
    {
        const auto activity = static_cast<Info*> (activityInfo);
        emitActivityUpdated(model, container, activity->id(), role);
    }

}

ActivitiesModelPrivate::ActivitiesModelPrivate(ActivitiesModel *parent)
    : q(parent)
{
}

ActivitiesModel::ActivitiesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ActivitiesModelPrivate(this))
{
    // Initializing role names for qml
    connect(&d->activities, &Consumer::serviceStatusChanged,
            this,           [this] (Consumer::ServiceStatus status) { d->setServiceStatus(status); });

    connect(&d->activities, &Consumer::activityAdded,
            this,           [this] (const QString &activity) { d->onActivityAdded(activity); });
    connect(&d->activities, &Consumer::activityRemoved,
            this,           [this] (const QString &activity) { d->onActivityRemoved(activity); });
    connect(&d->activities, &Consumer::currentActivityChanged,
            this,           [this] (const QString &activity) { d->onCurrentActivityChanged(activity); });

    d->setServiceStatus(d->activities.serviceStatus());
}

ActivitiesModel::~ActivitiesModel()
{
    delete d;
}

QHash<int, QByteArray> ActivitiesModel::roleNames() const
{
    return {
        {Qt::DisplayRole,     "name"},
        {Qt::DecorationRole,  "icon"},

        {ActivityState,       "state"},
        {ActivityId,          "id"},
        {ActivityIcon,        "iconSource"},
        {ActivityDescription, "description"},
        {ActivityBackground,  "background"},
        {ActivityIsCurrent,   "current"},
        {ActivityIsCurrent,   "isCurrent"}
    };
}


void ActivitiesModelPrivate::setServiceStatus(Consumer::ServiceStatus)
{
    replaceActivities(activities.activities());
}

void ActivitiesModelPrivate::replaceActivities(const QStringList &activities)
{
    model_reset m(q);

    knownActivities.clear();
    shownActivities.clear();

    for (const QString &activity: activities) {
        onActivityAdded(activity, false);
    }
}

void ActivitiesModelPrivate::onActivityAdded(const QString &id, bool notifyClients)
{
    auto info = registerActivity(id);

    showActivity(info, notifyClients);
}

void ActivitiesModelPrivate::onActivityRemoved(const QString &id)
{
    hideActivity(id);
    unregisterActivity(id);
}

void ActivitiesModelPrivate::onCurrentActivityChanged(const QString &id)
{
    Q_UNUSED(id);

    for (const auto &activity: shownActivities) {
        Private::emitActivityUpdated(this, shownActivities, activity->id(),
                                     ActivitiesModel::ActivityIsCurrent);
    }
}

ActivitiesModelPrivate::InfoPtr ActivitiesModelPrivate::registerActivity(const QString &id)
{
    auto position = Private::activityPosition(knownActivities, id);

    if (position) {
        return *(position->second);

    } else {
        auto activityInfo = std::make_shared<Info>(id);

        auto ptr = activityInfo.get();

        connect(ptr,  &Info::nameChanged,
                this, &ActivitiesModelPrivate::onActivityNameChanged);
        connect(ptr,  &Info::descriptionChanged,
                this, &ActivitiesModelPrivate::onActivityDescriptionChanged);
        connect(ptr,  &Info::iconChanged,
                this, &ActivitiesModelPrivate::onActivityIconChanged);
        connect(ptr,  &Info::stateChanged,
                this, &ActivitiesModelPrivate::onActivityStateChanged);

        knownActivities.insert(InfoPtr(activityInfo));

        return activityInfo;
    }
}

void ActivitiesModelPrivate::unregisterActivity(const QString &id)
{
    auto position = Private::activityPosition(knownActivities, id);

    if (position) {
        if (auto shown = Private::activityPosition(shownActivities, id)) {
            model_remove(q, QModelIndex(), shown->first,
                                  shown->first);
            shownActivities.erase(shown->second);
        }

        knownActivities.erase(position->second);
    }
}

void ActivitiesModelPrivate::showActivity(InfoPtr activityInfo, bool notifyClients)
{
    // Should it really be shown?
    if (!Private::matchingState(activityInfo, shownStates)) return;

    // Is it already shown?
    if (boost::binary_search(shownActivities, activityInfo,
                             InfoPtrComparator())) return;

    auto registeredPosition
        = Private::activityPosition(knownActivities, activityInfo->id());

    if (!registeredPosition) {
        qDebug() << "Got a request to show an unknown activity, ignoring";
        return;
    }

    auto activityInfoPtr = *(registeredPosition->second);

    auto position = shownActivities.insert(activityInfoPtr);

    if (notifyClients) {
        unsigned int index =
            (position.second ? position.first : shownActivities.end())
            - shownActivities.begin();

        model_insert(q, QModelIndex(), index, index);
    }
}

void ActivitiesModelPrivate::hideActivity(const QString &id)
{
    auto position = Private::activityPosition(shownActivities, id);

    if (position) {
        model_remove(q, QModelIndex(),
            position->first, position->first);
        shownActivities.erase(position->second);
    }
}

#define CREATE_SIGNAL_EMITTER(What, Role)                                      \
    void ActivitiesModelPrivate::onActivity##What##Changed(const QString &)             \
    {                                                                          \
        Private::emitActivityUpdated(this, shownActivities, sender(), Role); \
    }

CREATE_SIGNAL_EMITTER(Name, Qt::DisplayRole)
CREATE_SIGNAL_EMITTER(Description, ActivitiesModel::ActivityDescription)
CREATE_SIGNAL_EMITTER(Icon, Qt::DecorationRole)

#undef CREATE_SIGNAL_EMITTER

void ActivitiesModelPrivate::onActivityStateChanged(Info::State state)
{
    if (shownStates.empty()) {
        Private::emitActivityUpdated(this, shownActivities, sender(),
                                     ActivitiesModel::ActivityState);

    } else {
        auto info = findActivity(sender());

        if (!info) {
            return;
        }

        if (boost::binary_search(shownStates, state)) {
            showActivity(info, true);
        } else {
            hideActivity(info->id());
        }
    }
}

void ActivitiesModel::setShownStates(const QVector<Info::State> &states)
{
    d->shownStates = states;

    d->replaceActivities(d->activities.activities());

    emit shownStatesChanged(states);
}

QVector<Info::State> ActivitiesModel::shownStates() const
{
    return d->shownStates;
}

int ActivitiesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return d->shownActivities.size();
}

QVariant ActivitiesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const auto &item = *(d->shownActivities.cbegin() + row);

    switch (role) {
    case Qt::DisplayRole:
        return item->name();

    case Qt::DecorationRole:
        return QVariant(); // QIcon::fromTheme(data(index, ActivityIcon).toString());

    case ActivityId:
        return item->id();

    case ActivityState:
        return item->state();

    case ActivityIcon:
        {
            const QString &icon = item->icon();

            // We need a default icon for activities
            return icon.isEmpty() ? "preferences-activities" : icon;
        }

    case ActivityDescription:
        return item->description();

    case ActivityIsCurrent:
        return d->activities.currentActivity() == item->id();

    default:
        return QVariant();
    }
}

QVariant ActivitiesModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    Q_UNUSED(orientation);

    // if (section == 0 && role == Qt::DisplayRole) {
    //     return i18nc("Header title for activity data model", "Activity");
    // }

    return QVariant();
}

ActivitiesModelPrivate::InfoPtr ActivitiesModelPrivate::findActivity(QObject *ptr) const
{
    auto info = boost::find_if(knownActivities, [ptr] (const InfoPtr &info) {
            return ptr == info.get();
        });

    if (info == knownActivities.end()) {
        return nullptr;
    } else {
        return *info;
    }
}

} // namespace KActivities

// #include "activitiesmodel.moc"

