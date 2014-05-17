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

// KDE
#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfileitem.h>

// Local
#include "utils/continue_with.h"

using kamd::utils::continue_with;

namespace KActivities {
namespace Models {

ResourceModel::ResourceModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_shownActivity(QStringLiteral(":current"))
    , m_shownAgent(QStringLiteral(":current"))
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


} // namespace Models
} // namespace KActivities

#include "resourcemodel.moc"
