/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "BlacklistedApplicationsModel.h"

#include <QList>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include <KStandardDirs>
#include <KService>
#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>

#include <utils/d_ptr_implementation.h>
#include <utils/val.h>


class BlacklistedApplicationsModel::Private {
public:
    struct ApplicationData {
        QString name;
        QString title;
        QString icon;
        bool blocked;
    };

    QList <ApplicationData> applications;
    QSqlDatabase database;

    KSharedConfig::Ptr pluginConfig;
    bool enabled;
};

BlacklistedApplicationsModel::BlacklistedApplicationsModel(QObject * parent)
    : QAbstractListModel(parent)
{
    QHash < int, QByteArray > roles;
    roles[ApplicationIdRole]       = "name";
    roles[Qt::DecorationRole]      = "icon";
    roles[Qt::DisplayRole]         = "title";
    roles[BlockedApplicationRole]  = "blocked";
    setRoleNames(roles);

    d->enabled = false;
    d->pluginConfig = KSharedConfig::openConfig("activitymanager-pluginsrc");
}

void BlacklistedApplicationsModel::load()
{
    // Loading plugin configuration

    val config = d->pluginConfig->group("Plugin-org.kde.kactivitymanager.resourcescoring");

    val defaultBlockedValue = config.readEntry("blocked-by-default", false);
    auto blockedApplications = QSet<QString>::fromList(config.readEntry("blocked-applications", QStringList()));
    auto allowedApplications = QSet<QString>::fromList(config.readEntry("allowed-applications", QStringList()));

    // Reading new applications from the database

    val path = KStandardDirs::locateLocal("data", "activitymanager/resources/database", true);

    d->database = QSqlDatabase::addDatabase("QSQLITE", "plugins_sqlite_db_resources");
    d->database.setDatabaseName(path);

    if (!d->database.open()) {
        qDebug() << "Failed to open the database" << path << d->database.lastError();
        return;
    }

    auto query = d->database.exec("SELECT DISTINCT(initiatingAgent) FROM kext_ResourceScoreCache ORDER BY initiatingAgent");

    if (d->applications.length() > 0) {
        beginRemoveRows(QModelIndex(), 0, d->applications.length() - 1);
        d->applications.clear();
        endRemoveRows();
    }

    while (query.next()) {
        val name = query.value(0).toString();

        if (defaultBlockedValue) {
            if (!allowedApplications.contains(name))
                blockedApplications << name;
        } else {
            if (!blockedApplications.contains(name))
                allowedApplications << name;
        }
    }

    auto applications = (blockedApplications + allowedApplications).toList();

    if (applications.length() > 0) {
        qSort(applications);

        beginInsertRows(QModelIndex(), 0, applications.length() - 1);

        foreach (val & name, applications) {
            val service = KService::serviceByDesktopName(name);
            val blocked = blockedApplications.contains(name);

            if (service) {
                d->applications << Private::ApplicationData {
                    name,
                        service->name(),
                        service->icon(),
                        blocked
                };
            } else {
                d->applications << Private::ApplicationData { name, name, name, blocked };
            }
        }

        endInsertRows();
    }
}

void BlacklistedApplicationsModel::save()
{
    auto config = d->pluginConfig->group("Plugin-org.kde.kactivitymanager.resourcescoring");
    QStringList blockedApplications;
    QStringList allowedApplications;

    for (int i = 0; i < rowCount(); i++) {
        (d->applications[i].blocked ? blockedApplications : allowedApplications)
            << d->applications[i].name;
    }

    config.writeEntry("allowed-applications", allowedApplications);
    config.writeEntry("blocked-applications", blockedApplications);
}

void BlacklistedApplicationsModel::defaults()
{
    for (int i = 0; i < rowCount(); i++) {
        d->applications[i].blocked = false;
    }

    dataChanged(QAbstractListModel::index(0),
                QAbstractListModel::index(rowCount() - 1));
}

void BlacklistedApplicationsModel::toggleApplicationBlocked(int index)
{
    if (index > rowCount())
        return;

    d->applications[index].blocked = !d->applications[index].blocked;
    dataChanged(QAbstractListModel::index(index),
                QAbstractListModel::index(index));

    emit changed();
}

QVariant BlacklistedApplicationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

QVariant BlacklistedApplicationsModel::data(const QModelIndex & modelIndex, int role) const
{
    val index = modelIndex.row();

    if (index > rowCount())
        return QVariant();

    val & application = d->applications[index];

    switch (role) {
        default:
            return QVariant();

        case ApplicationIdRole:
            return application.name;

        case Qt::DisplayRole:
            return application.title;

        case Qt::DecorationRole:
            return application.icon;

        case BlockedApplicationRole:
            return application.blocked;
    }
}

int BlacklistedApplicationsModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->applications.size();
}

bool BlacklistedApplicationsModel::enabled() const
{
    return d->enabled;
}

void BlacklistedApplicationsModel::setEnabled(bool enabled)
{
    d->enabled = enabled;
    emit enabledChanged(enabled);
}

#include <BlacklistedApplicationsModel.moc>
