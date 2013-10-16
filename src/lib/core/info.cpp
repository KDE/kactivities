/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "info.h"
#include "info_p.h"
#include "manager_p.h"
#include "dbusfuture_p.h"

#include <QFileSystemWatcher>

namespace KActivities {

// Private

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
    void InfoPrivate::INTERNAL(const QString &_id, const QString &val) const   \
    { if (id == _id) emit q->INTERNAL(val); }

IMPLEMENT_SIGNAL_HANDLER(nameChanged)
IMPLEMENT_SIGNAL_HANDLER(iconChanged)

#undef IMPLEMENT_SIGNAL_HANDLER

void InfoPrivate::activityStateChanged(const QString &idChanged,
                                       int newState) const
{
    if (idChanged == id) {
        emit q->stateChanged(static_cast<Info::State>(newState));
    }
}

// Info
Info::Info(const QString &activity, QObject *parent)
    : QObject(parent)
    , d(new InfoPrivate(this, activity))
{
#define PASS_SIGNAL_HANDLER(SIGNAL_NAME, SLOT_NAME)                            \
    connect(d->cache.data(), SIGNAL(SIGNAL_NAME(QString)),                     \
            this,            SLOT(SLOT_NAME(QString)));

    PASS_SIGNAL_HANDLER(activityAdded, added)
    PASS_SIGNAL_HANDLER(activityRemoved, removed)
    // PASS_SIGNAL_HANDLER(started)
    // PASS_SIGNAL_HANDLER(stopped)
    PASS_SIGNAL_HANDLER(activityChanged, infoChanged)
    // PASS_SIGNAL_HANDLER(nameChanged)
    // PASS_SIGNAL_HANDLER(iconChanged)
    PASS_SIGNAL_HANDLER(activityStateChanged, activityStateChanged)

#undef PASS_SIGNAL_HANDLER
}

Info::~Info()
{
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

Info::State Info::state() const
{
    if (d->cache->m_status == Consumer::Unknown) return Info::Unknown;

    auto info = d->cache->cfind(d->id);

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

        if (Manager::features()->IsFeatureOperational(QStringLiteral("org.kde.ActivityManager.Nepomuk/linking"))) {
            result = Everything;
        }
    }

    return result;
}

QString Info::name() const
{
    auto info = d->cache->cfind(d->id);

    return info ? info->name : QString();
}

QString Info::icon() const
{
    auto info = d->cache->cfind(d->id);

    return info ? info->icon : QString();
}

void Info::linkResource(const QString &resourceUri)
{
    Manager::resourcesLinking()->LinkResourceToActivity(resourceUri, d->id);
}

void Info::unlinkResource(const QString &resourceUri)
{
    Manager::resourcesLinking()->UnlinkResourceFromActivity(resourceUri, d->id);
}

QFuture<bool> Info::isResourceLinked(const QString &resourceUri)
{
    // return Manager::resourcesLinking()->IsResourceLinkedToActivity(resourceUri, d->id);
    return DBusFuture::asyncCall<bool>(
            Manager::resourcesLinking(),
            QStringLiteral("IsResourceLinkedToActivity"),
            resourceUri,
            d->id);
}

} // namespace KActivities

#include "moc_info.cpp"
#include "moc_info_p.cpp"
