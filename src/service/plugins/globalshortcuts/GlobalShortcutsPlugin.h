/*
 *   Copyright (C) 2012 Makis Marimpis <makhsm@gmail.com>
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

#ifndef PLUGINS_GLOBAL_SHORTCUTS_PLUGIN_H
#define PLUGINS_GLOBAL_SHORTCUTS_PLUGIN_H

#include <Plugin.h>

#include <utils/override.h>

class QSignalMapper;
class KActionCollection;

class GlobalShortcutsPlugin: public Plugin
{
    Q_OBJECT

public:
    GlobalShortcutsPlugin(QObject * parent, const QVariantList & args);
    virtual ~GlobalShortcutsPlugin();

    virtual bool init(const QHash < QString, QObject * > & modules) _override;

private Q_SLOTS:
    void activityAdded(const QString & activity);
    void activityRemoved(const QString & activity);
    void activityChanged(const QString & activity);

private:
    inline QString activityName(const QString & activity) const;

    QObject * m_activitiesService;
    QSignalMapper * m_signalMapper;
    KActionCollection * m_actionCollection;
};

#endif // PLUGINS_GLOBAL_SHORTCUTS_PLUGIN_H
