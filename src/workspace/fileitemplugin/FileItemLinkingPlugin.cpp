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

#include "FileItemLinkingPlugin.h"
#include "FileItemLinkingPlugin_p.h"

#include <kfileitemlistproperties.h>
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery.h>

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
#include <QDBusInterface>
#include <QDBusPendingCall>

#include <KPluginFactory>
#include <KLocalizedString>

K_PLUGIN_FACTORY_WITH_JSON(ActivityLinkingFileItemActionFactory,
                           "kactivitymanagerd_fileitem_linking_plugin.json",
                           registerPlugin<FileItemLinkingPlugin>();)


FileItemLinkingPlugin::Private::Private()
    : shouldLoad(false)
{
    connect(&activities, &KActivities::Consumer::serviceStatusChanged,
            this, &Private::activitiesServiceStatusChanged);
}

void FileItemLinkingPlugin::Private::activitiesServiceStatusChanged(
    KActivities::Consumer::ServiceStatus status)
{
    if (status != KActivities::Consumer::Unknown) {
        loadAllActions();
    }
}

void FileItemLinkingPlugin::Private::rootActionHovered()
{
    shouldLoad = true;
    loadAllActions();
}

void FileItemLinkingPlugin::Private::actionTriggered()
{
    QAction *action = dynamic_cast<QAction *>(sender());

    if (!action) {
        return;
    }

    bool link = action->property("link").toBool();
    QString activity = action->property("activity").toString();

    QDBusInterface service("org.kde.ActivityManager",
                           "/ActivityManager/Resources/Linking",
                           "org.kde.ActivityManager.ResourcesLinking");

    foreach (const auto &item, items.urlList())
    {
        if (link) {
            service.asyncCall("LinkResourceToActivity", QString(),
                    item.toLocalFile(), activity);

        } else {
            service.asyncCall("UnlinkResourceFromActivity", QString(),
                    item.toLocalFile(), activity);

        }
    }
}

FileItemLinkingPlugin::FileItemLinkingPlugin(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
}

FileItemLinkingPlugin::~FileItemLinkingPlugin()
{
    d->setActions({});
}

QAction *FileItemLinkingPlugin::Private::basicAction(QWidget *parentWidget)
{
    root = new QAction(QIcon::fromTheme("preferences-activities"),
                                i18n("Activities"), parentWidget);

    rootMenu = new QMenu();
    rootMenu->addAction(new QAction(i18n("Loading..."), this));

    connect(root, &QAction::hovered,
            this, &Private::rootActionHovered);


    root->setMenu(rootMenu);

    return root;
}

QList<QAction *> FileItemLinkingPlugin::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    d->items = fileItemInfos;

    return { d->basicAction(parentWidget) };
}

void FileItemLinkingPlugin::Private::loadAllActions()
{
    if (!shouldLoad
        || activities.serviceStatus() == KActivities::Consumer::Unknown) {
        return;
    }

    if (activities.serviceStatus() == KActivities::Consumer::NotRunning) {
        setActions({ new QAction(
            i18n("The Activity Manager is not running"), Q_NULLPTR) });

    } else {
        auto loader = new FileItemLinkingPluginActionLoader(items);

        static FileItemLinkingPluginActionStaticInit init;

        connect(loader, &FileItemLinkingPluginActionLoader::result,
                this, &Private::setActions);

        loader->start();
    }
}

void FileItemLinkingPlugin::Private::setActions(const QList<QAction*> &actions)
{
    for (auto action: rootMenu->actions()) {
        rootMenu->removeAction(action);
        action->deleteLater();
    }

    for (auto action: actions) {
        rootMenu->addAction(action);
        connect(action, &QAction::triggered,
                this, &Private::actionTriggered);
    }
}

FileItemLinkingPluginActionLoader::FileItemLinkingPluginActionLoader(
    const KFileItemListProperties &items)
    : items(items)
{
}

void FileItemLinkingPluginActionLoader::run()
{
    QList<QAction*> actions;

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
                    "AND initiatingAgent = \"\" "
                    "AND usedActivity != \"\" "
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

    deleteLater();
}

QAction *
FileItemLinkingPluginActionLoader::createAction(const QString &activity,
                                                bool link, const QString &title,
                                                const QString &icon) const
{
    auto action = new QAction(Q_NULLPTR);

    if (title.isEmpty()) {
        KActivities::Info activityInfo(activity);
        action->setText(activityInfo.name());
        action->setIcon(QIcon::fromTheme(activityInfo.icon().isEmpty()
                                             ? "preferences-activities"
                                             : activityInfo.icon()));

    } else {
        action->setText(title);
    }

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    action->setProperty("activity", activity.isEmpty()
                                        ? activities.currentActivity()
                                        : activity);
    action->setProperty("link", link);

    action->setVisible(true);

    return action;
}

QAction *
FileItemLinkingPluginActionLoader::createSeparator(const QString &title) const
{
    auto action = new QAction(title, Q_NULLPTR);
    action->setSeparator(true);
    return action;
}

FileItemLinkingPluginActionStaticInit::FileItemLinkingPluginActionStaticInit()
{
    qRegisterMetaType<ActionList>("ActionList");
}

#include "FileItemLinkingPlugin.moc"

