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

#ifndef FILE_ITEM_LINKING_PLUGIN_P_H
#define FILE_ITEM_LINKING_PLUGIN_P_H

#include "FileItemLinkingPlugin.h"

#include <QThread>
#include <QUrl>

#include <KFileItemListProperties>

#include "lib/core/consumer.h"
#include "lib/core/info.h"

typedef QList<QAction *> ActionList;

class FileItemLinkingPlugin::Private : public QObject {
    Q_OBJECT

public:
    Private();

    QAction *root;
    QMenu *rootMenu;
    KFileItemListProperties items;

    QAction *basicAction(QWidget *parentWidget);

    KActivities::Consumer activities;

public Q_SLOTS:
    void activitiesServiceStatusChanged(KActivities::Consumer::ServiceStatus status);
    void rootActionHovered();
    void setActions(const ActionList &actions);

    void actionTriggered();
    void loadAllActions();

private:
    bool shouldLoad : 1;
};

class FileItemLinkingPluginActionLoader: public QThread {
    Q_OBJECT

public:
    FileItemLinkingPluginActionLoader(const KFileItemListProperties &items);

    void run() Q_DECL_OVERRIDE;

    QAction *createAction(const QString &activity, bool link,
                          const QString &title = QString(),
                          const QString &icon = QString()) const;
    QAction *createSeparator(const QString &title) const;

Q_SIGNALS:
    void result(const ActionList &actions);

private:
    KFileItemListProperties items;
    KActivities::Consumer activities;
};

class FileItemLinkingPluginActionStaticInit {
public:
    FileItemLinkingPluginActionStaticInit();
};

#endif // FILE_ITEM_LINKING_PLUGIN_P_H
