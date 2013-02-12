/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PLUGINS_VIRTUAL_DESKTOP_SWITCH_PLUGIN_H
#define PLUGINS_VIRTUAL_DESKTOP_SWITCH_PLUGIN_H

#include <Plugin.h>

#include <utils/override.h>

class VirtualDesktopSwitchPlugin: public Plugin
{
    Q_OBJECT

public:
    VirtualDesktopSwitchPlugin(QObject * parent, const QVariantList & args);
    virtual ~VirtualDesktopSwitchPlugin();

    virtual bool init(const QHash < QString, QObject * > & modules) _override;

private Q_SLOTS:
    void currentActivityChanged(const QString & activity);
    void activityRemoved(const QString & activity);

private:
    QString m_currentActivity;
    QObject * m_activitiesService;
};

#endif // PLUGINS_VIRTUAL_DESKTOP_SWITCH_PLUGIN_H
