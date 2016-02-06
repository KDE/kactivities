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

K_PLUGIN_FACTORY_WITH_JSON(ActivityLinkingFileItemActionFactory,
                           "kactivitymanagerd_fileitem_linking_plugin.json",
                           registerPlugin<FileItemLinkingPlugin>();)


// Private

FileItemLinkingPlugin::Private::Private()
    : shouldLoad(false)
    , loaded(false)
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

    KAMD_DBUS_DECL_INTERFACE(service, Resources/Linking, ResourcesLinking);

    foreach (const auto &item, items.urlList()) {
        service.asyncCall(
                link ? "LinkResourceToActivity" : "UnlinkResourceFromActivity",
                QString(),
                item.toLocalFile(),
                activity);
    }
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

void FileItemLinkingPlugin::Private::loadAllActions()
{
    if (!shouldLoad
        || activities.serviceStatus() == KActivities::Consumer::Unknown) {
        return;
    }

    if (activities.serviceStatus() == KActivities::Consumer::NotRunning) {
        Action action = { };
        action.title = i18n("The Activity Manager is not running");

        setActions({ action });

    } else if (!loaded) {
        auto loader = FileItemLinkingPluginActionLoader::create(items);

        static FileItemLinkingPluginActionStaticInit init;

        connect(loader, &FileItemLinkingPluginActionLoader::result,
                this, &Private::setActions,
                Qt::QueuedConnection);

        loader->start();

        loaded = true; // ignore that the thread may not be finished at this time
    }
}

void FileItemLinkingPlugin::Private::setActions(const ActionList &actions)
{
    for (auto action: rootMenu->actions()) {
        rootMenu->removeAction(action);
        action->deleteLater();
    }

    for (auto actionInfo: actions) {
        if (actionInfo.icon != "-") {
            auto action = new QAction(Q_NULLPTR);

            action->setText(actionInfo.title);
            action->setIcon(QIcon::fromTheme(actionInfo.icon));
            action->setProperty("activity", actionInfo.activity);
            action->setProperty("link", actionInfo.link);

            rootMenu->addAction(action);

            connect(action, &QAction::triggered,
                    this, &Private::actionTriggered);

        } else {
            auto action = new QAction(actionInfo.title, Q_NULLPTR);
            action->setSeparator(true);

            rootMenu->addAction(action);
        }
    }
}

FileItemLinkingPluginActionStaticInit::FileItemLinkingPluginActionStaticInit()
{
    qRegisterMetaType<Action>("Action");
    qRegisterMetaType<ActionList>("ActionList");
}

// Main class

FileItemLinkingPlugin::FileItemLinkingPlugin(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
}

FileItemLinkingPlugin::~FileItemLinkingPlugin()
{
    d->setActions({});
}

QList<QAction *> FileItemLinkingPlugin::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    d->items = fileItemInfos;

    return { d->basicAction(parentWidget) };
}

#include "FileItemLinkingPlugin.moc"

