/*
 *   Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>
 *
 *   based on work by:
 *   Copyright (C) 2007 Thomas Georgiou <TAGeorgiou@gmail.com> and Jeff Cooper <weirdsox11@gmail.com>
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

#ifndef NEPOMUK_H
#define NEPOMUK_H

#include <QObject>
#include <KUrl>

#include <Nepomuk/Resource>

#include "../../Plugin.h"
#include "Rankings.h"

class NepomukPlugin: public Plugin {
public:
    NepomukPlugin(QObject *parent = 0, const QVariantList & args = QVariantList());

    virtual void addEvents(const EventList & events);

    static NepomukPlugin * self();

    bool init();

private:
    Nepomuk::Resource createDesktopEvent(const KUrl& uri, const QDateTime& startTime, const QString& app);

    Nepomuk::Resource m_currentActivity;
    Rankings * m_rankings;

    static NepomukPlugin * s_instance;
};

#endif
