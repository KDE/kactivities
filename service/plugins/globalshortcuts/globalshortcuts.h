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

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <KActionCollection>
#include <KAction>
#include <KGlobalSettings>
#include <KLocalizedString>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QSignalMapper>
#include <QList>

#include "../../Plugin.h"

class GlobalShortcutsPlugin: public Plugin
{
    Q_OBJECT

public:
    GlobalShortcutsPlugin(QObject *parent, const QVariantList & args);
    ~GlobalShortcutsPlugin();

    virtual bool init();

private slots:
    void realInit();
    void listActivitiesFinished(QDBusPendingCallWatcher * watcher);
    void updateNextActivityName(QDBusPendingCallWatcher * watcher);
    void setCurrentActivity(const QString & activity);

private:
    QDBusInterface* m_dbusInterface;
    QStringList m_activities;
    QSignalMapper* m_signalMapper;
    QList<KAction*> m_actions;
    KActionCollection* m_actionCollection;
};

#endif
