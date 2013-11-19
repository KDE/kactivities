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
#include <QString>
#include <QStringList>

// Utils
#include <utils/d_ptr.h>

// Local
#include <Debug.h>

class QDateTime;
class QSqlDatabase;

class Database : public QObject {
    Q_OBJECT

public:
    static Database *self();

    template <typename T>
    inline QSqlQuery exec(const T &query)
    {
        return database()->exec(query);
    }

    template <typename T1, typename T2, typename... Ts>
    inline QSqlQuery exec(const T1 &query1, const T2 &query2,
                          const Ts &... queries)
    {
        exec(query1);
        return exec(query2, queries...);
    }


    void openDesktopEvent(const QString &usedActivity,
                          const QString &initiatingAgent,
                          const QString &targettedResource,
                          const QDateTime &start,
                          const QDateTime &end = QDateTime());

    void closeDesktopEvent(const QString &usedActivity,
                           const QString &initiatingAgent,
                           const QString &targettedResource,
                           const QDateTime &end);

    void getResourceScoreCache(const QString &usedActivity,
                               const QString &initiatingAgent,
                               const QString &targettedResource, qreal &score,
                               QDateTime &lastUpdate);

private:
    QSqlDatabase *database();

    Database();
    ~Database();

    void initDatabaseSchema();

    D_PTR;
};

#endif // PLUGINS_SQLITE_DATABASE_CONNECTION_H
