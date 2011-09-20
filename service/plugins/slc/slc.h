/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SLC_H
#define SLC_H

#include <QObject>
#include <KUrl>

#include "../../Plugin.h"

class SlcPlugin: public Plugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.SLC")

public:
    SlcPlugin(QObject *parent = 0, const QVariantList & args = QVariantList());
    ~SlcPlugin();

    virtual void addEvents(const EventList & events);

private Q_SLOTS:
    void activeWindowChanged(WId windowId);

public Q_SLOTS:
    QString focussedResourceURI();
    QString focussedResourceMimetype();
    QString focussedResourceTitle();

Q_SIGNALS:
    void focusChanged(const QString & uri, const QString & mimetype, const QString & title);

private:
    void updateFocus(WId wid = 0);

    WId focussedWindow;
    KUrl _focussedResourceURI();
    QHash < WId, KUrl > lastFocussedResource;
};

#endif
