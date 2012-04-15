/*
 *   Copyright (C) 2012 Makis Marimpis makhsm@gmail.com
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

#include "globalshortcuts.h"

#include <KDebug>


GlobalShortcutsPlugin::GlobalShortcutsPlugin(QObject * parent, const QVariantList & args)
    : Plugin(parent),
      m_signalMapper(new QSignalMapper(this)),
      m_actionCollection(new KActionCollection(this))
{
    Q_UNUSED(args)
}

bool GlobalShortcutsPlugin::init()
{
    // All plugins are loaded from ActivityManager's ctor and this one
    // specifically makes use of some functionality (e.g. listing all activities
    // via dbus) that is provided after ActivityManager's ctor is called.
    // So we queue the real inialization in a slot, for invokation
    // at a later time - when the application enter the main event loop.
    QMetaObject::invokeMethod(this, "realInit", Qt::QueuedConnection);

    return true;
}

GlobalShortcutsPlugin::~GlobalShortcutsPlugin()
{
}

void GlobalShortcutsPlugin::realInit()
{
    m_dbusInterface = new QDBusInterface("org.kde.kactivitymanagerd",
                                         "/ActivityManager",
                                         "org.kde.ActivityManager",
                                          QDBusConnection::sessionBus(),
                                          this);

    QDBusPendingCall call = m_dbusInterface->asyncCall("ListActivities");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(listActivitiesFinished(QDBusPendingCallWatcher*)));
}

void GlobalShortcutsPlugin::listActivitiesFinished(QDBusPendingCallWatcher * watcher)
{
    QDBusPendingReply<QStringList> reply = *watcher;

    if (reply.isValid()) {
        m_activities = reply.argumentAt<0>();
        updateNextActivityName(0);
    }

    watcher->deleteLater();
}

void GlobalShortcutsPlugin::updateNextActivityName(QDBusPendingCallWatcher * watcher)
{
    if (watcher) {
        QDBusPendingReply<QString> reply = *watcher;

        if (reply.isValid()) {
            KAction * action = m_actionCollection->addAction(QString("switch-to-activity-%1").arg(watcher->property("activity").toString()));
            action->setText(i18nc("@action", "Switch to activity \"%1\"", reply.value()));
            action->setGlobalShortcut(KShortcut());
            connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
            m_signalMapper->setMapping(action, watcher->property("activity").toString());
        }

        watcher->deleteLater();
    }

    if (m_activities.isEmpty()) {
        connect(m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(setCurrentActivity(QString)));
        m_actionCollection->readSettings();
        return;
    }

    QString activity = m_activities.takeFirst();
    QDBusPendingReply<QString> reply = m_dbusInterface->asyncCall("ActivityName", activity);

    QDBusPendingCallWatcher* watcher2 = new QDBusPendingCallWatcher(reply, this);
    watcher2->setProperty("activity", activity);
    connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(updateNextActivityName(QDBusPendingCallWatcher*)));
}

void GlobalShortcutsPlugin::setCurrentActivity(const QString & activity)
{
    m_dbusInterface->asyncCall("SetCurrentActivity", activity);
}

KAMD_EXPORT_PLUGIN(GlobalShortcutsPlugin, "activitymanger_plugin_globalshortcuts")
