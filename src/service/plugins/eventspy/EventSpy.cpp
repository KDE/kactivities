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

#include "EventSpy.h"

#include <QStringList>
#include <QString>
#include <QFileInfo>

#include <KIOCore/KRecentDocument>
#include <KCoreAddons/KDirWatch>
#include <KConfigCore/KDesktopFile>
#include <KConfigCore/KConfigGroup>

KAMD_EXPORT_PLUGIN(eventspyplugin, EventSpyPlugin, "kactivitymanagerd-plugin-eventspy.json")

EventSpyPlugin::EventSpyPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_resources(Q_NULLPTR)
    , m_dirWatcher(new KDirWatch())
    , m_lastUpdate(QDateTime::currentDateTime())
{
    Q_UNUSED(args);

    m_dirWatcher->addDir(KRecentDocument::recentDocumentDirectory());

    connect(m_dirWatcher.get(), &KDirWatch::dirty,
            this, &EventSpyPlugin::directoryUpdated);
}

void EventSpyPlugin::directoryUpdated(const QString &dir)
{
    Q_UNUSED(dir);
    const auto newDocuments = KRecentDocument::recentDocuments();

    // Processing the new arrivals
    for (const auto& document: newDocuments) {
        QFileInfo fileInfo(document);
        if (fileInfo.lastModified() > m_lastUpdate) {
            addDocument(document);
        }
    }

    m_lastUpdate = QDateTime::currentDateTime();
}

void EventSpyPlugin::addDocument(const QString &document)
{
    const KDesktopFile desktopFile(document);
    const KConfigGroup desktopGroup(&desktopFile, "Desktop Entry");

    const QString url = QUrl(desktopFile.readUrl()).toLocalFile();
    const QString name = desktopFile.readName();
    const QString application
        = desktopGroup.readEntry("X-KDE-LastOpenedWith", QString());

    Plugin::invoke<Qt::QueuedConnection>(
        m_resources, "RegisterResourceEvent",
                Q_ARG(QString, application), // Application
                Q_ARG(uint, 0),              // Window ID
                Q_ARG(QString, url),         // URI
                Q_ARG(uint, 0)               // Event Activities::Accessed
        );
}

EventSpyPlugin::~EventSpyPlugin()
{
}

bool EventSpyPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_resources = modules["resources"];

    return true;
}

#include "EventSpy.moc"

