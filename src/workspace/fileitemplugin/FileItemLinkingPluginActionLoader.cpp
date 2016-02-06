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

#include "FileItemLinkingPluginActionLoader.h"

#include <kfileitemlistproperties.h>
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery_iterator.h>

#include <QMenu>
#include <QCursor>
#include <QDebug>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QDBusPendingCall>

#include <KPluginFactory>
#include <KLocalizedString>

#include "common/dbus/common.h"

FileItemLinkingPluginActionLoader* FileItemLinkingPluginActionLoader::create(const KFileItemListProperties &items)
{
    auto l = new FileItemLinkingPluginActionLoader(items);
    connect(l, &QThread::finished, l, &QObject::deleteLater);
    return l;
}

FileItemLinkingPluginActionLoader::FileItemLinkingPluginActionLoader(
    const KFileItemListProperties &items)
    : items(items)
{
}

void FileItemLinkingPluginActionLoader::run()
{
    ActionList actions;

    const auto activitiesList = activities.activities();
    const auto itemsSize = items.urlList().size();

    if (itemsSize >= 10) {
        // we are not going to check for this large number of files
        actions << createAction(QString(), true,
                                i18n("Link to the current activity"),
                                "list-add");
        actions << createAction(QString(), false,
                                i18n("Unlink from the current activity"),
                                "list-remove");

        actions << createSeparator(i18n("Link to:"));
        for (const auto& activity: activitiesList) {
            actions << createAction(activity, true);
        }

        actions << createSeparator(i18n("Unlink from:"));
        for (const auto& activity: activitiesList) {
            actions << createAction(activity, false);
        }

    } else {
        auto database = QSqlDatabase::addDatabase(
            QStringLiteral("QSQLITE"),
            QStringLiteral("kactivities_db_resources_")
            + QString::number((quintptr) this));

        database.setDatabaseName(
            QStandardPaths::writableLocation(
                QStandardPaths::GenericDataLocation)
            + QStringLiteral("/kactivitymanagerd/resources/database"));

        if (database.open()) {

            static const auto queryString = QStringLiteral(
                "SELECT usedActivity, COUNT(targettedResource) "
                "FROM ResourceLink "
                "WHERE targettedResource IN (%1) "
                    "AND initiatingAgent = \":global\" "
                    "AND usedActivity != \":global\" "
                "GROUP BY usedActivity");

            QStringList escapedFiles;
            QSqlField field;
            field.setType(QVariant::String);

            for (const auto& item: items.urlList()) {
                field.setValue(QFileInfo(item.toLocalFile()).canonicalFilePath());
                escapedFiles << database.driver()->formatValue(field);
            }

            QSqlQuery query(queryString.arg(escapedFiles.join(",")),
                            database);

            QStringList activitiesForLinking;
            QStringList activitiesForUnlinking;

            for (const auto& result: query) {
                const auto linkedFileCount = result[1].toInt();
                const auto activity = result[0].toString();
                if (linkedFileCount < itemsSize) {
                    activitiesForLinking << activity;
                }

                if (linkedFileCount > 0) {
                    activitiesForUnlinking << activity;
                }
            }

            if (activitiesForLinking.contains(activities.currentActivity()) ||
                    !activitiesForUnlinking.contains(activities.currentActivity())) {
                actions << createAction(QString(), true,
                                        i18n("Link to the current activity"),
                                        "list-add");
            }
            if (activitiesForUnlinking.contains(activities.currentActivity())) {
                actions << createAction(QString(), false,
                                        i18n("Unlink from the current activity"),
                                        "list-remove");
            }

            actions << createSeparator(i18n("Link to:"));
            for (const auto& activity: activitiesList) {
                if (activitiesForLinking.contains(activity) ||
                        !activitiesForUnlinking.contains(activity)) {
                    actions << createAction(activity, true);
                }
            }

            actions << createSeparator(i18n("Unlink from:"));
            for (const auto& activity: activitiesList) {
                if (activitiesForUnlinking.contains(activity)) {
                    actions << createAction(activity, false);
                }
            }
        }
    }

    emit result(actions);
}

Action
FileItemLinkingPluginActionLoader::createAction(const QString &activity,
                                                bool link, const QString &title,
                                                const QString &icon) const
{
    Action action = { };
    action.link = link;

    if (title.isEmpty()) {
        KActivities::Info activityInfo(activity);
        action.title = activityInfo.name();
        action.icon  = activityInfo.icon().isEmpty() ? "preferences-activities"
                                                     : activityInfo.icon();

    } else {
        action.title = title;
    }

    if (!icon.isEmpty()) {
        action.icon = icon;
    }

    action.activity = activity.isEmpty() ? activities.currentActivity()
                                         : activity;

    return action;
}

Action
FileItemLinkingPluginActionLoader::createSeparator(const QString &title) const
{
    Action action = { };
    action.icon = "-";
    action.title = title;
    return action;
}

