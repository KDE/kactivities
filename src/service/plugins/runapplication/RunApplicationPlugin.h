/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef PLUGINS_RUN_APPLICATION_H
#define PLUGINS_RUN_APPLICATION_H

#include <Plugin.h>

class RunApplicationPlugin : public Plugin {
    Q_OBJECT
    // Q_PLUGIN_METADATA(IID "org.kde.ActivityManager.plugins.virtualdesktopswitch")

public:
    RunApplicationPlugin(QObject *parent = Q_NULLPTR, const QVariantList &args = QVariantList());
    virtual ~RunApplicationPlugin();

    bool init(QHash<QString, QObject *> &modules) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void currentActivityChanged(const QString &activity);
    void activityStateChanged(const QString &activity, int state);

private:
    QString activityDirectory(const QString &activity) const;
    void executeIn(const QString &directory) const;

    QString m_currentActivity;
    QStringList m_previousActivities;
    QObject *m_activitiesService;
};

#endif // PLUGINS_RUN_APPLICATION_H
