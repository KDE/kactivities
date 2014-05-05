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

// Boost
#include <boost/range/algorithm/binary_search.hpp>
#include <utils/range.h>

// Local
#include "Debug.h"
#include "Database.h"
#include "Utils.h"
#include "resourcelinkingadaptor.h"

ResourceLinking::ResourceLinking(QObject *parent)
    : QObject(parent)
{
    new ResourcesLinkingAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
        QStringLiteral("/ActivityManager/Resources/Linking"), this);

}

void ResourceLinking::LinkResourceToActivity(const QString &initiatingAgent,
                                             const QString &targettedResource,
                                             const QString &usedActivity)
{
    qDebug() << "agent" << initiatingAgent
             << "resource" << targettedResource
             << "activity" << usedActivity;

    Utils::prepare(Database::self()->database(), linkResourceToActivityQuery, QStringLiteral(
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
}

void ResourceLinking::UnlinkResourceFromActivity(const QString &initiatingAgent,
                                                 const QString &targettedResource,
                                                 const QString &usedActivity)
{
    Utils::prepare(Database::self()->database(), unlinkResourceFromActivityQuery, QStringLiteral(
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
}

bool ResourceLinking::IsResourceLinkedToActivity(const QString &initiatingAgent,
                                                 const QString &targettedResource,
                                                 const QString &usedActivity)
{
    Utils::prepare(Database::self()->database(), isResourceLinkedToActivityQuery, QStringLiteral(
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

