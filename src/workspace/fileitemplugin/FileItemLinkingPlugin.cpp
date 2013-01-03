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

#include "FileItemLinkingPlugin.h"
#include "FileItemLinkingPlugin_p.h"

#include <kfileitemlistproperties.h>

#include <QMenu>
#include <QDebug>

#include <KLocale>
#include <KPluginFactory>

void FileItemLinkingPlugin::Private::actionTriggered()
{
    QAction * action = dynamic_cast < QAction * > (sender());

    if (!action) return;

    bool    link     = action->property("link").toBool();
    QString activity = action->property("activity").toString();

    qDebug() << activity << link;

    foreach (const KUrl & item, items) {
        if (link) {
            activities.linkResourceToActivity(item, activity);

        } else {
            activities.unlinkResourceFromActivity(item, activity);

        }
    }

}

QAction * FileItemLinkingPlugin::Private::addAction(QMenu * menu,
        const QString & activityId, const QString & title, const QString & icon)
{
    QAction * action = menu->addAction(
            title.isEmpty() ? KActivities::Info::name(activityId) : title
        );

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    action->setProperty("activity", activityId);

    connect(action, SIGNAL(triggered()),
            this, SLOT(actionTriggered()));

    return action;
}

FileItemLinkingPlugin::FileItemLinkingPlugin(QObject * parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent), d(new Private())
{
}

FileItemLinkingPlugin::~FileItemLinkingPlugin()
{
    delete d;
}

QList <QAction *> FileItemLinkingPlugin::actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget)
{
    QList < QAction * > result;

    QAction * root = new QAction(QIcon::fromTheme("preferences-activities"), i18n("Activities"), parentWidget);
    QMenu * rootMenu = new QMenu();
    root->setMenu(rootMenu);

    d->items = fileItemInfos.urlList();
    const unsigned itemCount = d->items.size();
    if (itemCount == 0) {
        return result;

    } else {

        bool haveLinked = false;
        bool haveUnlinked = false;

        foreach (const KUrl & url, d->items) {
            (d->activities.isResourceLinkedToActivity(url) ? haveLinked : haveUnlinked) = true;
        }

        if (haveLinked) {
            d->addAction(rootMenu, QString(), i18n("Unlink from the current activity"), "list-remove")
                ->setProperty("link", false);
        }

        if (haveUnlinked) {
            d->addAction(rootMenu, QString(), i18n("Link to the current activity"), "list-add")
                ->setProperty("link", true);
        }

        QStringList linkable, unlinkable;

        foreach (const QString & activity, d->activities.listActivities()) {
            haveLinked = haveUnlinked = false;

            foreach (const KUrl & url, d->items) {
                (d->activities.isResourceLinkedToActivity(url, activity) ? haveLinked : haveUnlinked) = true;
            }

            if (haveLinked) {
                unlinkable << activity;
            }

            if (haveUnlinked) {
                linkable << activity;
            }
        }

        rootMenu->addSeparator()->setText(i18n("Link to:"));
        foreach (const QString & activity, linkable) {
            d->addAction(rootMenu, activity)
                ->setProperty("link", true);
        }

        rootMenu->addSeparator()->setText(i18n("Unlink from:"));
        foreach (const QString & activity, unlinkable) {
            d->addAction(rootMenu, activity)
                ->setProperty("link", false);
        }

    }

    return result << root;
}

K_PLUGIN_FACTORY(FileItemLinkingPluginFactory, registerPlugin<FileItemLinkingPlugin>();)
K_EXPORT_PLUGIN(FileItemLinkingPluginFactory("kactivitymanagerd_fileitem_linking_plugin"))

