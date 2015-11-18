/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <QSignalMapper>
#include <QAction>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>

KAMD_EXPORT_PLUGIN(globalshortcutsplugin, GlobalShortcutsPlugin, "kactivitymanagerd-plugin-globalshortcuts.json")

const auto objectNamePattern = QStringLiteral("switch-to-activity-%1");
const auto objectNamePatternLength = objectNamePattern.length() - 2;

GlobalShortcutsPlugin::GlobalShortcutsPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_activitiesService(Q_NULLPTR)
    , m_signalMapper(new QSignalMapper(this))
    , m_actionCollection(new KActionCollection(this))
{
    Q_UNUSED(args);
}

GlobalShortcutsPlugin::~GlobalShortcutsPlugin()
{
    m_actionCollection->clear();
}

bool GlobalShortcutsPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_activitiesService = modules["activities"];

    m_activitiesList = Plugin::retrieve<QStringList>(
            m_activitiesService, "ListActivities", "QStringList");

    for (const auto &activity: m_activitiesList) {
        activityAdded(activity);
    }

    connect(m_signalMapper, SIGNAL(mapped(QString)),
            m_activitiesService, SLOT(SetCurrentActivity(QString)),
            Qt::QueuedConnection);
    connect(m_activitiesService, SIGNAL(ActivityAdded(QString)),
            this, SLOT(activityAdded(QString)));
    connect(m_activitiesService, SIGNAL(ActivityRemoved(QString)),
            this, SLOT(activityRemoved(QString)));

    m_actionCollection->readSettings();

    activityRemoved();

    return true;
}

void GlobalShortcutsPlugin::activityAdded(const QString &activity)
{
    if (activity == "00000000-0000-0000-0000-000000000000") {
        return;
    }

    if (!m_activitiesList.contains(activity)) {
        m_activitiesList << activity;
    }

    const auto action = m_actionCollection->addAction(
        objectNamePattern.arg(activity));

    action->setText(i18nc("@action", "Switch to activity \"%1\"", activityName(activity)));
    KGlobalAccel::self()->setGlobalShortcut(action, QList<QKeySequence>{});

    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, activity);

    m_actionCollection->writeSettings();
}

QString GlobalShortcutsPlugin::activityForAction(QAction *action) const
{
    return action->objectName().mid(objectNamePatternLength);
}

void GlobalShortcutsPlugin::activityRemoved(const QString &deletedActivity)
{
    m_activitiesList.removeAll(deletedActivity);

    // Removing all shortcuts that refer to an unknown activity
    for (const auto &action: m_actionCollection->actions()) {
        const auto actionActivity = activityForAction(action);
        if ((deletedActivity.isEmpty() && !m_activitiesList.contains(actionActivity))
                || deletedActivity == actionActivity) {
            KGlobalAccel::self()->removeAllShortcuts(action);
            m_actionCollection->removeAction(action);
        }
    }

    m_actionCollection->writeSettings();
}

void GlobalShortcutsPlugin::activityChanged(const QString &activity)
{
    for (const auto &action: m_actionCollection->actions()) {
        if (activity == activityForAction(action)) {
            action->setText(i18nc("@action", "Switch to activity \"%1\"",
                                  activityName(activity)));
        }
    }
}

QString GlobalShortcutsPlugin::activityName(const QString &activity) const
{
    return Plugin::retrieve<QString>(
        m_activitiesService, "ActivityName", "QString",
        Q_ARG(QString, activity));
}

#include "GlobalShortcutsPlugin.moc"

