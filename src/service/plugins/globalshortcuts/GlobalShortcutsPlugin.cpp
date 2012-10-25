/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic@kde.org>
 *   Copyright (C) 2012 Makis Marimpis <makhsm@gmail.com>
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

#include "GlobalShortcutsPlugin.h"

#include <QStringList>
#include <QString>

#include <KAction>

#include <utils/nullptr.h>
#include <utils/val.h>

val objectNamePattern       = QString::fromLatin1("switch-to-activity-%1");
val objectNamePatternLength = objectNamePattern.length() - 2;

GlobalShortcutsPlugin::GlobalShortcutsPlugin(QObject * parent, const QVariantList & args)
    : Plugin(parent),
      m_activitiesService(nullptr),
      m_signalMapper(new QSignalMapper(this)),
      m_actionCollection(new KActionCollection(this))
{
    Q_UNUSED(args)
}

GlobalShortcutsPlugin::~GlobalShortcutsPlugin()
{
    m_actionCollection->clear();
}

bool GlobalShortcutsPlugin::init(const QHash < QString, QObject * > & modules)
{
    m_activitiesService = modules["activities"];

    val activitiesList = Plugin::callOn <QStringList, Qt::DirectConnection> (m_activitiesService, "ListActivities", "QStringList");

    foreach (val & activityId, activitiesList) {
        activityAdded(activityId);
    }

    connect(
        m_signalMapper, SIGNAL(mapped(QString)),
        m_activitiesService, SLOT(SetCurrentActivity(QString)),
        Qt::QueuedConnection
    );

    m_actionCollection->readSettings();

    foreach (val action, m_actionCollection->actions()) {
        if (!activitiesList.contains(action->objectName().mid(objectNamePatternLength))) {
            m_actionCollection->removeAction(action);
        }
    }

    m_actionCollection->writeSettings();

    return true;
}

void GlobalShortcutsPlugin::activityAdded(const QString & activityId)
{
    val action = m_actionCollection->addAction(
            objectNamePattern.arg(activityId)
        );

    action->setText(i18nc("@action", "Switch to activity \"%1\"", activityName(activityId)));
    action->setGlobalShortcut(KShortcut());

    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, activityId);

    m_actionCollection->writeSettings();
}

void GlobalShortcutsPlugin::activityRemoved(const QString & activityId)
{
    foreach (val action, m_actionCollection->actions()) {
        if (activityId == action->objectName().mid(objectNamePatternLength)) {
            m_actionCollection->removeAction(action);
        }
    }

    m_actionCollection->writeSettings();
}

void GlobalShortcutsPlugin::activityChanged(const QString & activityId)
{
    foreach (val action, m_actionCollection->actions()) {
        if (activityId == action->objectName().mid(objectNamePatternLength)) {
            action->setText(i18nc("@action", "Switch to activity \"%1\"", activityName(activityId)));
        }
    }
}

QString GlobalShortcutsPlugin::activityName(const QString & activityId) const
{
    return Plugin::callOnWithArgs <QString, Qt::DirectConnection> (
            m_activitiesService, "ActivityName", "QString",
            Q_ARG(QString, activityId)
        );
}

KAMD_EXPORT_PLUGIN(GlobalShortcutsPlugin, "activitymanager_plugin_globalshortcuts")

