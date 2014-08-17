/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <kactivities-features.h>
#include "ResourceLinking.h"

// Qt
#include <QFileSystemWatcher>
#include <QSqlQuery>

// KDE
#include <kconfig.h>
#include <kdbusconnectionpool.h>
#include <kdirnotify.h>

// Boost
#include <boost/range/algorithm/binary_search.hpp>
#include <utils/range.h>

// Local
#include "Debug.h"
#include "Database.h"
#include "Utils.h"
#include "StatsPlugin.h"
#include "resourcelinkingadaptor.h"

ResourceLinking::ResourceLinking(QObject *parent)
    : QObject(parent)
{
    new ResourcesLinkingAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
        QStringLiteral("/ActivityManager/Resources/Linking"), this);

    connect(&m_activities, &KActivities::Consumer::currentActivityChanged,
            this, &ResourceLinking::onCurrentActivityChanged);
    connect(&m_activities, &KActivities::Consumer::activityAdded,
            this, &ResourceLinking::onActivityAdded);
    connect(&m_activities, &KActivities::Consumer::activityRemoved,
            this, &ResourceLinking::onActivityRemoved);
}

void ResourceLinking::LinkResourceToActivity(QString initiatingAgent,
                                             QString targettedResource,
                                             QString usedActivity)
{
    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        qDebug() << "Invalid arguments" << initiatingAgent << targettedResource
                 << usedActivity;
        return;
    }

    Utils::prepare(Database::self()->database(), linkResourceToActivityQuery,
        QStringLiteral(
            "INSERT OR REPLACE INTO ResourceLink"
            "        (usedActivity,  initiatingAgent,  targettedResource) "
            "VALUES ( "
                "COALESCE(:usedActivity,''),"
                "COALESCE(:initiatingAgent,''),"
                "COALESCE(:targettedResource,'')"
            ")"
        ));

    Utils::exec(*linkResourceToActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    if (!usedActivity.isEmpty()) {
        org::kde::KDirNotify::emitFilesAdded(QStringLiteral("activities:/")
                                             + usedActivity);
    }

    emit ResourceLinkedToActivity(initiatingAgent, targettedResource,
                                  usedActivity);
}

void ResourceLinking::UnlinkResourceFromActivity(QString initiatingAgent,
                                                 QString targettedResource,
                                                 QString usedActivity)
{
    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        qDebug() << "Invalid arguments" << initiatingAgent << targettedResource
                 << usedActivity;
        return;
    }

    Utils::prepare(Database::self()->database(), unlinkResourceFromActivityQuery,
        QStringLiteral(
            "DELETE FROM ResourceLink "
            "WHERE "
            "usedActivity      = COALESCE(:usedActivity     , '') AND "
            "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
            "targettedResource = COALESCE(:targettedResource, '') "
        ));

    Utils::exec(*unlinkResourceFromActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    if (!usedActivity.isEmpty()) {
        QString mangled = targettedResource;
        mangled.replace("/", "_");
        org::kde::KDirNotify::emitFilesRemoved(
            { QStringLiteral("activities:/") + usedActivity + '/' + mangled });
    }

    emit ResourceUnlinkedFromActivity(initiatingAgent, targettedResource,
                                      usedActivity);
}

bool ResourceLinking::IsResourceLinkedToActivity(QString initiatingAgent,
                                                 QString targettedResource,
                                                 QString usedActivity)
{
    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        return false;
    }

    Utils::prepare(Database::self()->database(), isResourceLinkedToActivityQuery,
        QStringLiteral(
            "SELECT * FROM ResourceLink "
            "WHERE "
            "usedActivity      = COALESCE(:usedActivity     , '') AND "
            "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
            "targettedResource = COALESCE(:targettedResource, '') "
        ));

    Utils::exec(*isResourceLinkedToActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    return isResourceLinkedToActivityQuery->next();
}

bool ResourceLinking::validateArguments(QString &initiatingAgent,
                                        QString &targettedResource,
                                        QString &usedActivity)
{
    Q_UNUSED(initiatingAgent)

    // Validating targetted resource
    if (targettedResource.startsWith(QStringLiteral("file://"))) {
        targettedResource = QUrl(targettedResource).toLocalFile();
    }

    if (targettedResource.startsWith(QStringLiteral("/"))) {
        QFileInfo file(targettedResource);

        if (!file.exists()) {
            return false;
        }

        targettedResource = file.canonicalFilePath();
    }

    // If the activity is not empty and the passed activity
    // does not exist, cancel the request
    if (!usedActivity.isEmpty()
        && !Plugin::callOn<QStringList, Qt::DirectConnection>(
                StatsPlugin::self()->activitiesInterface(),
                "ListActivities", "QStringList").contains(usedActivity)) {
        return false;
    }

    // qDebug() << "agent" << initiatingAgent
    //          << "resource" << targettedResource
    //          << "activity" << usedActivity;

    return true;
}

void ResourceLinking::onActivityAdded(const QString &activity)
{
    // Notify KIO
    qDebug() << "Added: " << activity;
    org::kde::KDirNotify::emitFilesAdded(QStringLiteral("activities:/"));
}

void ResourceLinking::onActivityRemoved(const QString &activity)
{
    // Notify KIO
    qDebug() << "Removed: " << activity;
    org::kde::KDirNotify::emitFilesRemoved(
        { QStringLiteral("activities:/") + activity });

    // Remove statistics for the activity
}

void ResourceLinking::onCurrentActivityChanged(const QString &activity)
{
    // Notify KIO
    qDebug() << "Changed: " << activity;
    org::kde::KDirNotify::emitFilesChanged(
        { QStringLiteral("activities:/current") });
}

