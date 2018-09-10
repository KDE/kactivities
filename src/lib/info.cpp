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

#include "info.h"
#include "info_p.h"
#include "manager_p.h"

#include "utils/dbusfuture_p.h"

#include <QFileSystemWatcher>

namespace KActivities {

// InfoPrivate

InfoPrivate::InfoPrivate(Info *info, const QString &activity)
    : q(info)
    , cache(ActivitiesCache::self())
    , id(activity)
{
}

// Filters out signals for only this activity
#define IMPLEMENT_SIGNAL_HANDLER(INTERNAL)                                     \
    void InfoPrivate::INTERNAL(const QString &_id) const                       \
    {   if (id == _id) emit q->INTERNAL(); }

IMPLEMENT_SIGNAL_HANDLER(added)
IMPLEMENT_SIGNAL_HANDLER(removed)
IMPLEMENT_SIGNAL_HANDLER(started)
IMPLEMENT_SIGNAL_HANDLER(stopped)
IMPLEMENT_SIGNAL_HANDLER(infoChanged)

#undef IMPLEMENT_SIGNAL_HANDLER

#define IMPLEMENT_SIGNAL_HANDLER(INTERNAL)                                     \
    void InfoPrivate::INTERNAL##Changed(const QString &_id,                    \
                                        const QString &val) const              \
    {                                                                          \
        if (id == _id) {                                                       \
            emit q->INTERNAL##Changed(val);                                    \
        }                                                                      \
    }

IMPLEMENT_SIGNAL_HANDLER(name)
IMPLEMENT_SIGNAL_HANDLER(description)
IMPLEMENT_SIGNAL_HANDLER(icon)

#undef IMPLEMENT_SIGNAL_HANDLER

void InfoPrivate::activityStateChanged(const QString &idChanged,
                                       int newState) const
{
    if (idChanged == id) {
        auto state = static_cast<Info::State>(newState);
        emit q->stateChanged(state);

        if (state == KActivities::Info::Stopped) {
            emit q->stopped();
        } else if (state == KActivities::Info::Running) {
            emit q->started();
        }
    }
}

void InfoPrivate::setCurrentActivity(const QString &currentActivity)
{
    if (isCurrent) {
        if (currentActivity != id) {
            // We are no longer the current activity
            isCurrent = false;
            emit q->isCurrentChanged(false);
        }
    } else {
        if (currentActivity == id) {
            // We are the current activity
            isCurrent = true;
            emit q->isCurrentChanged(true);
        }
    }
}

// Info
Info::Info(const QString &activity, QObject *parent)
    : QObject(parent)
    , d(new InfoPrivate(this, activity))
{
    // qDebug() << "Created an instance of Info: " << (void*)this;
#define PASS_SIGNAL_HANDLER(SIGNAL_NAME,SLOT_NAME)                            \
    connect(d->cache.get(),  SIGNAL(SIGNAL_NAME(QString)),                     \
            this,            SLOT(SLOT_NAME(QString)));

    PASS_SIGNAL_HANDLER(activityAdded,added)
    PASS_SIGNAL_HANDLER(activityRemoved,removed)
    // PASS_SIGNAL_HANDLER(started)
    // PASS_SIGNAL_HANDLER(stopped)
    PASS_SIGNAL_HANDLER(activityChanged,infoChanged)
#undef PASS_SIGNAL_HANDLER

#define PASS_SIGNAL_HANDLER(SIGNAL_NAME,SLOT_NAME,TYPE)                      \
    connect(d->cache.get(),  SIGNAL(SIGNAL_NAME(QString,TYPE)),               \
            this,            SLOT(SLOT_NAME(QString,TYPE)));                  \

    PASS_SIGNAL_HANDLER(activityStateChanged,activityStateChanged,int);
    PASS_SIGNAL_HANDLER(activityNameChanged,nameChanged,QString);
    PASS_SIGNAL_HANDLER(activityDescriptionChanged,descriptionChanged,QString);
    PASS_SIGNAL_HANDLER(activityIconChanged,iconChanged,QString);
#undef PASS_SIGNAL_HANDLER

    connect(d->cache.get(),  SIGNAL(currentActivityChanged(QString)),
            this,            SLOT(setCurrentActivity(QString)));

    d->isCurrent = (d->cache.get()->m_currentActivity == activity);
}

Info::~Info()
{
    // qDebug() << "Deleted an instance of Info: " << (void*)this;
}

bool Info::isValid() const
{
    auto currentState = state();
    return (currentState != Invalid && currentState != Unknown);
}

QString Info::uri() const
{
    return QStringLiteral("activities://") + d->id;
}

QString Info::id() const
{
    return d->id;
}

bool Info::isCurrent() const
{
    return d->isCurrent;
}

Info::State Info::state() const
{
    if (d->cache->m_status == Consumer::Unknown) return Info::Unknown;

    auto info = d->cache->getInfo(d->id);

    if (!info) return Info::Invalid;

    return static_cast<Info::State> (info->state);
}

void InfoPrivate::setServiceStatus(Consumer::ServiceStatus status) const
{
    switch (status) {
        case Consumer::NotRunning:
        case Consumer::Unknown:
            activityStateChanged(id, Info::Unknown);
            break;

        default:
            activityStateChanged(id, q->state());
            break;

    }
}

Info::Availability Info::availability() const
{
    Availability result = Nothing;

    if (!Manager::isServiceRunning()) {
        return result;
    }

    if (Manager::activities()->ListActivities().value().contains(d->id)) {
        result = BasicInfo;

        if (Manager::features()->IsFeatureOperational(QStringLiteral("resources/linking"))) {
            result = Everything;
        }
    }

    return result;
}

#define CREATE_GETTER(What)                                                    \
    QString Info::What() const                                                 \
    {                                                                          \
        auto info = d->cache->getInfo(d->id);                                  \
        return info ? info->What : QString();                                  \
    }

CREATE_GETTER(name)
CREATE_GETTER(description)
CREATE_GETTER(icon)

#undef CREATE_GETTER

} // namespace KActivities

#include "moc_info.cpp"
// #include "moc_info_p.cpp"
