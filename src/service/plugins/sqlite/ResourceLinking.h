/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGINS_SQLITE_RESOURCE_LINKING_H
#define PLUGINS_SQLITE_RESOURCE_LINKING_H

// Qt
#include <QObject>

// Boost and STL
#include <memory>
#include <boost/container/flat_set.hpp>

// Local
#include <Plugin.h>

class QSqlQuery;
class QFileSystemWatcher;

/**
 * Communication with the outer world.
 *
 * - Handles configuration
 * - Filters the events based on the user's configuration.
 */
class ResourceLinking : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources.Linking")

public:
    ResourceLinking(QObject *parent);

public Q_SLOTS:
    void LinkResourceToActivity(const QString &initiatingAgent,
                                const QString &targettedResource,
                                const QString &usedActivity = QString());
    void UnlinkResourceFromActivity(const QString &initiatingAgent,
                                    const QString &targettedResource,
                                    const QString &usedActivity = QString());
    bool IsResourceLinkedToActivity(const QString &initiatingAgent,
                                    const QString &targettedResource,
                                    const QString &usedActivity = QString());

private:
    std::unique_ptr<QSqlQuery> linkResourceToActivityQuery;
    std::unique_ptr<QSqlQuery> unlinkResourceFromActivityQuery;
    std::unique_ptr<QSqlQuery> isResourceLinkedToActivityQuery;

};

#endif // PLUGINS_SQLITE_RESOURCE_LINKING_H
