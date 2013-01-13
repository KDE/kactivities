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

#include <QObject>
#include <QUrl>
#include <QDateTime>

#include <utils/d_ptr.h>

class QDateTime;
class OUrl;
class QSqlDatabase;

class DatabaseConnection: public QObject {
    Q_OBJECT

public:
    static DatabaseConnection * self();

    void openDesktopEvent(const QString & usedActivity, const QString & initiatingAgent,
            const QString & targettedResource, const QDateTime & start, const QDateTime & end = QDateTime());
    void closeDesktopEvent(const QString & usedActivity, const QString & initiatingAgent,
            const QString & targettedResource, const QDateTime & end);

    void getResourceScoreCache(const QString & usedActivity, const QString & initiatingAgent,
            const QUrl & targettedResource, qreal & score, QDateTime & lastUpdate);

    QSqlDatabase & database();


private:
    static DatabaseConnection * s_instance;

    DatabaseConnection();
    ~DatabaseConnection();

    void initDatabaseSchema();

    D_PTR;
};

#endif // PLUGINS_SQLITE_DATABASE_CONNECTION_H

