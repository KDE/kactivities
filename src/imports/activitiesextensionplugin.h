/*
 *   Copyright (C) 2011, 2012 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITIES_COMPONENT_PLUGIN_H
#define ACTIVITIES_COMPONENT_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitiesExtensionPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.activities")

public:
    ActivitiesExtensionPlugin(QObject *parent = Q_NULLPTR);
    void registerTypes(const char *uri) Q_DECL_OVERRIDE;
};

#endif // ACTIVITIES_COMPONENT_DATA_PLUGIN_H
