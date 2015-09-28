/*
 *   Copyright (C) 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "plugin.h"

#include <QtQml>

#include "activitysettings.h"

#include <QDebug>

ActivitiesSettingsPlugin::ActivitiesSettingsPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

static QJSValue settingsSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)

    auto result = new ActivitySettings();
    return scriptEngine->newQObject(result);
}

void ActivitiesSettingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.activities.settings"));

    qmlRegisterSingletonType("org.kde.activities.settings", 0, 1,
                             "ActivitySettings",
                             settingsSingleton);
}

