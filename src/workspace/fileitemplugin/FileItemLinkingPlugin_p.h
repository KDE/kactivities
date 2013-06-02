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

#ifndef FILE_ITEM_LINKING_PLUGIN_P_H
#define FILE_ITEM_LINKING_PLUGIN_P_H

#include "FileItemLinkingPlugin.h"
#include <kabstractfileitemactionplugin.h>

#include <QThread>

#include <KUrl>

#include "lib/core/consumer.h"
#include "lib/core/info.h"

class FileItemLinkingPlugin::Private: public QObject {
    Q_OBJECT

public:
    KActivities::Consumer activities;
    KUrl::List items;
    QMenu * rootMenu;
    QThread * thread;

public Q_SLOTS:
    void actionTriggered();
    void showActions();

    void addAction(
            const QString & activity,
            bool link,
            const QString & title = QString(),
            const QString & icon = QString()
            );

    void addSeparator(const QString & title);

    void finishedLoading();

};

class FileItemLinkingPluginLoader: public QThread {
    Q_OBJECT

public:
    FileItemLinkingPluginLoader(
            QObject * parent, const KUrl::List & items);

Q_SIGNALS:
    void requestAction(
            const QString & activity,
            bool link,
            const QString & title = QString(),
            const QString & icon = QString()
        );

    void requestSeparator(const QString & title);

    void finishedLoading();

protected:
    void run(); //override

public:
    KActivities::Consumer activities;
    KUrl::List m_items;
};


#endif // FILE_ITEM_LINKING_PLUGIN_P_H

