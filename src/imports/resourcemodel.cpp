/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Self
#include "resourcemodel.h"

// Qt
#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QModelIndex>
#include <QCoreApplication>
#include <QDBusInterface>

// KDE
#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfileitem.h>

// STL
#include <mutex>

// Local
#include "utils/continue_with.h"

using kamd::utils::continue_with;

namespace KActivities {
namespace Models {

class ResourceModel::LinkerService: public QDBusInterface {
private:
    LinkerService()
        : QDBusInterface("org.kde.ActivityManager",
                         "/ActivityManager/Resources/Linking",
                         "org.kde.ActivityManager.ResourcesLinking")
    {
    }

public:
    static std::shared_ptr<LinkerService> self()
    {
        static std::weak_ptr<LinkerService> s_instance;
        static std::mutex singleton;

        std::lock_guard<std::mutex> singleton_lock(singleton);

        auto result = s_instance.lock();

        if (s_instance.expired()) {
            result.reset(new LinkerService());
            s_instance = result;
        }

        return std::move(result);
    }

};

ResourceModel::ResourceModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_shownActivity(QStringLiteral(":current"))
    , m_shownAgent(QStringLiteral(":current"))
    , m_linker(LinkerService::self())
{
    // TODO: What to do if the file does not exist?

    const QString databaseDir
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
          + QStringLiteral("/kactivitymanagerd/resources/");

    const QString databaseFile = databaseDir + QStringLiteral("database");

    m_database = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"),
        QStringLiteral("kactivities_db_resources"));

    m_database.setDatabaseName(databaseFile);

    m_database.open();

    m_databaseModel = new QSqlTableModel(parent, m_database);
    m_databaseModel->setTable("ResourceLink");
    m_databaseModel->select();

    setSourceModel(m_databaseModel);

    connect(&m_service, &KActivities::Consumer::currentActivityChanged,
            this, &ResourceModel::setCurrentActivity);

    connect(m_linker.get(), SIGNAL(ResourceLinkedToActivity(QString, QString, QString)),
            this, SLOT(resourceLinkedToActivity(QString, QString, QString)));
    connect(m_linker.get(), SIGNAL(ResourceUnlinkedFromActivity(QString, QString, QString)),
            this, SLOT(resourceUnlinkedFromActivity(QString, QString, QString)));
}

ResourceModel::~ResourceModel()
{
}

QHash<int, QByteArray> ResourceModel::roleNames() const
{
    return {
        { Qt::DisplayRole,    "name" },
        { Qt::DecorationRole, "icon" },
        { Resource,           "resource" },
        { Agent,              "agent" },
        { Activity,           "activity" }
    };
}

void ResourceModel::setShownActivity(const QString &activityId)
{
    m_shownActivity = activityId;
    reloadData();
}

QString ResourceModel::shownActivity() const
{
    return m_shownActivity;
}

void ResourceModel::setShownAgent(const QString &agent)
{
    m_shownAgent = agent;

    reloadData();
}

QString ResourceModel::shownAgent() const
{
    return m_shownAgent;
}

void ResourceModel::reloadData()
{
    const QString whereActivity = QStringLiteral("usedActivity=") + (
        m_shownActivity == ":current" ? "'" + m_service.currentActivity() + "'" :
        m_shownActivity == ":any"     ? "usedActivity" :
        m_shownActivity == ":global"  ? "''" :
                                        "'" + m_shownActivity + "'"
    );

    const QString whereAgent = QStringLiteral("initiatingAgent=") + (
        m_shownAgent == ":current" ? "'" + QCoreApplication::applicationName() + "'" :
        m_shownAgent == ":any"     ? "initiatingAgent" :
        m_shownAgent == ":global"  ? "''" :
                                     "'" + m_shownAgent + "'"
    );

    m_databaseModel->setFilter(whereActivity + " AND " + whereAgent);
}

void ResourceModel::setCurrentActivity(const QString &activity)
{
    Q_UNUSED(activity)

    if (m_shownActivity == ":current") {
        reloadData();
    }
}

QVariant ResourceModel::data(const QModelIndex &proxyIndex, int role) const
{
    auto index = mapToSource(proxyIndex);

    if (role == Qt::DisplayRole || role == Qt::DecorationRole) {
        auto url = m_databaseModel->data(index.sibling(index.row(), 2), Qt::DisplayRole).toString();

        // TODO: Will probably need some more special handling -
        // for application:/ and a few more

        if (url.startsWith('/')) {
            url = QStringLiteral("file://") + url;
        }

        KFileItem file(url);

        return role == Qt::DisplayRole ? file.name() : file.iconName();
    }

    return m_databaseModel->data(index.sibling(index.row(),
            role == Resource ? 2 :
            role == Agent    ? 1 :
            role == Activity ? 0 :
                               3
        ), Qt::DisplayRole);
}

void ResourceModel::linkResourceToActivity(const QString &resource,
                                           const QJSValue &callback)
{
    Q_UNUSED(resource)
    Q_UNUSED(callback)
}

void ResourceModel::linkResourceToActivity(const QString &resource,
                                           const QString &activity,
                                           const QJSValue &callback)
{
    Q_UNUSED(resource)
    Q_UNUSED(activity)
    Q_UNUSED(callback)
}

void ResourceModel::linkResourceToActivity(const QString &agent,
                                           const QString &resource,
                                           const QString &activity,
                                           const QJSValue &callback)
{
    Q_UNUSED(agent)
    Q_UNUSED(resource)
    Q_UNUSED(activity)
    Q_UNUSED(callback)
}

void ResourceModel::unlinkResourceFromActivity(const QString &resource,
                                               const QJSValue &callback)
{
    Q_UNUSED(resource)
    Q_UNUSED(callback)
}

void ResourceModel::unlinkResourceFromActivity(const QString &resource,
                                               const QString &activity,
                                               const QJSValue &callback)
{
    Q_UNUSED(resource)
    Q_UNUSED(activity)
    Q_UNUSED(callback)
}

void ResourceModel::unlinkResourceFromActivity(const QString &agent,
                                               const QString &resource,
                                               const QString &activity,
                                               const QJSValue &callback)
{
    Q_UNUSED(agent)
    Q_UNUSED(resource)
    Q_UNUSED(activity)
    Q_UNUSED(callback)
}

void ResourceModel::resourceLinkedToActivity(const QString &initiatingAgent,
                                             const QString &targettedResource,
                                             const QString &usedActivity)
{
    Q_UNUSED(targettedResource)

    const bool shouldUpdate =
        // Testing whether the agent matches
        (
            // If the agent is not important
            m_shownAgent == ":any" ||
            // or we are listening for the changes for the current agent
            (m_shownAgent == ":current" && m_shownAgent == initiatingAgent) ||
            // or for links that are global, and not related to a specific agent
            (m_shownAgent == ":global" && initiatingAgent == "") ||
            // or we have a specific agent to listen for
            m_shownAgent == initiatingAgent
        ) &&
        // Testing whether the activity matches
        (
            // If the activity is not important
            m_shownActivity == ":any" ||
            // or we are listening for the changes for the current activity
            (m_shownActivity == ":current" && usedActivity == m_service.currentActivity()) ||
            // or we want the globally linked resources
            (m_shownActivity == ":global" && usedActivity == "") ||
            // or we have a specific activity in mind
            m_shownActivity == usedActivity
        );

    if (shouldUpdate) {
        reloadData();
    }
}

void ResourceModel::resourceUnlinkedFromActivity(const QString &initiatingAgent,
                                                 const QString &targettedResource,
                                                 const QString &usedActivity)
{
    // These are the same at the moment
    resourceLinkedToActivity(initiatingAgent, targettedResource, usedActivity);
}


} // namespace Models
} // namespace KActivities

#include "resourcemodel.moc"
