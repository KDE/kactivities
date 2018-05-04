/*
 *   Copyright (C) 2011, 2012, 2013, 2014, 2015 by Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef KACTIVITIES_IMPORTS_PLUGIN_H
#define KACTIVITIES_IMPORTS_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitiesExtensionPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.activities")

public:
    explicit ActivitiesExtensionPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // KACTIVITIES_IMPORTS_PLUGIN_H

