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
#include <QCursor>

#include <KLocale>
#include <KPluginFactory>

void FileItemLinkingPlugin::Private::actionTriggered()
{
    QAction * action = dynamic_cast < QAction * > (sender());

    if (!action) return;

    bool    link     = action->property("link").toBool();
    QString activity = action->property("activity").toString();

    foreach (const KUrl & item, items) {
        if (link) {
            activities.linkResourceToActivity(item, activity);

        } else {
            activities.unlinkResourceFromActivity(item, activity);

        }
    }
}

void FileItemLinkingPlugin::Private::addAction(
        const QString & activity,
        bool link,
        const QString & title,
        const QString & icon
    )
{
    QAction * action = rootMenu->addAction(
            title.isEmpty() ? KActivities::Info::name(activity) : title
        );

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    action->setProperty("activity", activity);
    action->setProperty("link", link);

    connect(action, SIGNAL(triggered()),
            this, SLOT(actionTriggered()));

    action->setVisible(false);
}

void FileItemLinkingPlugin::Private::addSeparator(
        const QString & title)
{
    QAction * separator = rootMenu->addSeparator();
    separator->setText(title);
    separator->setVisible(false);
}

FileItemLinkingPlugin::FileItemLinkingPlugin(QObject * parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent), d(new Private())
{
}

FileItemLinkingPlugin::~FileItemLinkingPlugin()
{
    if (!d->thread) delete d;
}

QList <QAction *> FileItemLinkingPlugin::actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget)
{
    QAction * root = new QAction(QIcon::fromTheme("preferences-activities"), i18n("Activities..."), parentWidget);

    connect(root, SIGNAL(triggered()),
            d, SLOT(showActions()));

    d->items = fileItemInfos.urlList();

    return QList < QAction * > () << root;
}

void FileItemLinkingPlugin::Private::showActions()
{
    thread = new FileItemLinkingPluginLoader(this, items);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));

    connect(thread, SIGNAL(requestAction(QString, bool, QString, QString)),
            this, SLOT(addAction(QString, bool, QString, QString)),
            Qt::QueuedConnection
        );
    connect(thread, SIGNAL(requestSeparator(QString)),
            this, SLOT(addSeparator(QString)),
            Qt::QueuedConnection
        );
    connect(thread, SIGNAL(finishedLoading()),
            this, SLOT(finishedLoading()),
            Qt::QueuedConnection
        );

    rootMenu = new QMenu();

    rootMenu->addAction("Loading...");

    rootMenu->popup(QCursor::pos());

    connect(rootMenu, SIGNAL(aboutToHide()),
            this, SLOT(deleteLater()));

    thread->start();
}

void FileItemLinkingPlugin::Private::finishedLoading()
{
    rootMenu->removeAction(rootMenu->actions()[0]);

    foreach (QAction * action, rootMenu->actions()) {
        action->setVisible(true);
    }

    rootMenu->popup(QCursor::pos());
}

FileItemLinkingPluginLoader::FileItemLinkingPluginLoader(
        QObject * parent, const KUrl::List & items)
    : QThread(/*parent*/), m_items(items)
{

}

void FileItemLinkingPluginLoader::run()
{
    const unsigned itemCount = m_items.size();

    if (itemCount > 10) {
        // we are not going to check for this large number of files
        emit requestAction(
                QString(),
                false,
                i18n("Unlink from the current activity"),
                "list-remove"
            );
        emit requestAction(
                QString(),
                true,
                i18n("Link to the current activity"),
                "list-add"
            );

        const QStringList activitiesList = activities.listActivities();

        emit requestSeparator(i18n("Link to:"));

        foreach (const QString & activity, activitiesList) {
            emit requestAction(activity, true);
        }

        emit requestSeparator(i18n("Unlink from:"));

        foreach (const QString & activity, activitiesList) {
            emit requestAction(activity, false);
        }

    } else if (itemCount != 0) {

        bool haveLinked = false;
        bool haveUnlinked = false;

        foreach (const KUrl & url, m_items) {
            (activities.isResourceLinkedToActivity(url) ? haveLinked : haveUnlinked) = true;
        }

        if (haveLinked) {
            emit requestAction(
                    QString(),
                    false,
                    i18n("Unlink from the current activity"),
                    "list-remove"
                );
        }

        if (haveUnlinked) {
            emit requestAction(
                    QString(),
                    true,
                    i18n("Link to the current activity"),
                    "list-add"
                );
        }

        QStringList linkable, unlinkable;

        foreach (const QString & activity, activities.listActivities()) {
            haveLinked = haveUnlinked = false;

            foreach (const KUrl & url, m_items) {
                (activities.isResourceLinkedToActivity(url, activity) ? haveLinked : haveUnlinked) = true;
            }

            if (haveLinked) {
                unlinkable << activity;
            }

            if (haveUnlinked) {
                linkable << activity;
            }
        }

        emit requestSeparator(i18n("Link to:"));

        foreach (const QString & activity, linkable) {
            emit requestAction(activity, true);
        }

        emit requestSeparator(i18n("Unlink from:"));

        foreach (const QString & activity, unlinkable) {
            emit requestAction(activity, false);
        }

    }

    emit finishedLoading();
}

K_PLUGIN_FACTORY(FileItemLinkingPluginFactory, registerPlugin<FileItemLinkingPlugin>();)
K_EXPORT_PLUGIN(FileItemLinkingPluginFactory("kactivitymanagerd_fileitem_linking_plugin"))

