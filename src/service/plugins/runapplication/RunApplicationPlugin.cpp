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

#include "RunApplicationPlugin.h"

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QProcess>
#include <QDesktopServices>

#include <Debug.h>

#include <KService>

namespace {
    enum ActivityState {
        Running = 2,
        Stopped = 4
    };
}

KAMD_EXPORT_PLUGIN(runapplicationplugin, RunApplicationPlugin, "kactivitymanagerd-plugin-runapplication.json")

RunApplicationPlugin::RunApplicationPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_activitiesService(Q_NULLPTR)
{
    Q_UNUSED(args);

    setName("org.kde.ActivityManager.RunApplication");
}

RunApplicationPlugin::~RunApplicationPlugin()
{
}

bool RunApplicationPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_activitiesService = modules["activities"];

    connect(m_activitiesService, SIGNAL(CurrentActivityChanged(QString)),
            this, SLOT(currentActivityChanged(QString)));
    connect(m_activitiesService, SIGNAL(ActivityStateChanged(QString, int)),
            this, SLOT(activityStateChanged(QString, int)));

    const auto currentActivity = Plugin::retrieve<QString>(
            m_activitiesService, "CurrentActivity", "QString");

    currentActivityChanged(currentActivity);

    return true;
}

QString RunApplicationPlugin::activityDirectory(const QString &activity) const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + QStringLiteral("/kactivitymanagerd/activities/") + activity + '/';
}

void RunApplicationPlugin::currentActivityChanged(const QString &activity)
{
    if (m_currentActivity == activity) {
        return;
    }

    if (!m_currentActivity.isEmpty()) {
        executeIn(activityDirectory(activity) + "deactivated");
    }

    m_currentActivity = activity;

    executeIn(activityDirectory(activity) + "activated");

    if (!m_previousActivities.contains(activity)) {
        // This is the first time we have switched
        // to this activity in the current session,
        // pretending it has just been started
        activityStateChanged(activity, Running);

        m_previousActivities << activity;
    }
}

void RunApplicationPlugin::activityStateChanged(const QString &activity, int state)
{
    auto directory = (state == Running) ? QStringLiteral("started")
                   : (state == Stopped) ? QStringLiteral("stopped")
                   : QString();

    if (directory == "") {
        return;
    }

    executeIn(activityDirectory(activity) + directory);
}

void RunApplicationPlugin::executeIn(const QString &path) const
{
    QDir directory(path);
    for (const auto& item: directory.entryList(QDir::Files)) {
        QString filePath = directory.filePath(item);
        KService service(filePath);
        if (service.isValid() && service.isApplication()) {
            qDebug() << "Starting: " << service.exec();
            QProcess::startDetached(service.exec());
        } else {
            qDebug() << "Openning file: " << QUrl::fromLocalFile(filePath);
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    }
}

// void RunApplicationPlugin::activityRemoved(const QString &activity)
// {
//     // TODO: Clean up the directory
// }

#include "RunApplicationPlugin.moc"

