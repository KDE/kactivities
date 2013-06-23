/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef RESOURCES_P_H
#define RESOURCES_P_H

#include "Resources.h"
#include "resourcesadaptor.h"

#include <QString>
#include <QList>
#include <KUrl>

class Resources::Private: public QThread {
    Q_OBJECT

public:
    Private(Resources * parent);

    void run();

    // Inserts the event directly into the queue
    void insertEvent(const Event & newEvent);

    // Processes the event and inserts it into the queue
    void addEvent(const QString & application, WId wid, const QString & uri,
                int type, int reason);

    // Processes the event and inserts it into the queue
    void addEvent(const Event & newEvent);

    QList <KUrl> resourcesLinkedToActivity(const QString & activity) const;

private Q_SLOTS:
    // Reacting to window manager signals
    void windowClosed(WId windowId);

    void activeWindowChanged(WId windowId);

private:
    struct WindowData {
        QSet < KUrl > resources;
        KUrl focussedResource;
        QString application;
    };

    Event lastEvent;
    // EventList events;
    // QMutex events_mutex;

    QHash < WId, WindowData > windows;
    WId focussedWindow;

    Resources * const q;
};

#endif // RESOURCES_P_H
