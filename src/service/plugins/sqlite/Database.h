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

#ifndef PLUGINS_SQLITE_RESOURCESDATABASE_H
#define PLUGINS_SQLITE_RESOURCESDATABASE_H

// Qt
#include <QObject>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QStringList>

// Utils
#include <utils/d_ptr.h>

// Local
#include <Debug.h>

class QDateTime;
class QSqlDatabase;
class QSqlError;

namespace Common {
    class Database;
} // namespace Common

class ResourcesDatabaseMigrator : public QObject {
    Q_OBJECT

public:
    // static Database *self();

private:
    ResourcesDatabaseMigrator();
    ~ResourcesDatabaseMigrator();

    void migrateDatabase(const QString &newDatabaseFile) const;

    D_PTR;

    friend Common::Database &resourcesDatabase();
};

Common::Database &resourcesDatabase();

#endif // PLUGINS_SQLITE_RESOURCESDATABASE_H
