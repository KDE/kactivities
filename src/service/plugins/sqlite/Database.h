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

#ifndef PLUGINS_SQLITE_DATABASE_CONNECTION_H
#define PLUGINS_SQLITE_DATABASE_CONNECTION_H

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

class Database : public QObject {
    Q_OBJECT

public:
    static Database *self();

    inline QSqlQuery addQuery()
    {
        return QSqlQuery(database());
    }

    QSqlDatabase &database();

private:
    Database();
    ~Database();

    void migrateDatabase(const QString &newDatabaseFile) const;
    void initDatabaseSchema();

    D_PTR;
};

#endif // PLUGINS_SQLITE_DATABASE_CONNECTION_H
