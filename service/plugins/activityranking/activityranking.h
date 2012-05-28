/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_H
#define PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_H

#include <QObject>
#include <QList>
#include <QString>

#include "../../Plugin.h"
#include "ActivityData.h"

class ActivityRankingPlugin: public Plugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.ActivityRanking")

public:
    explicit ActivityRankingPlugin(QObject *parent = 0, const QVariantList & args = QVariantList());
    ~ActivityRankingPlugin();

    virtual bool init();

public Q_SLOTS:
    /**
     * Lists top activities based on score calculation described in scoring.pdf.
     * All scores are related to the current location
     *
     * @return list of activities ids.
     */
    QStringList topActivities();

    /**
     * @return list of activities data with their rank in the context of the current location.
     */
    QList<ActivityData> activities();
    void test();

Q_SIGNALS:
    void rankingChanged(const QStringList & topActivities, const ActivityDataList & activities);

protected:
    void initDatabaseSchema();

private Q_SLOTS:
    void activityChanged(const QString & activity);
    void locationChanged(const QString & location);

private:
    class Private;
    Private * const d;
};

#endif // PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_H

