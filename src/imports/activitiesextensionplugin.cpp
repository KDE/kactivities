/*
    SPDX-FileCopyrightText: 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activitiesextensionplugin.h"

#include "activityinfo.h"
#include "activitymodel.h"
#include "resourceinstance.h"

// #include "resourcemodel.h"

// TODO: Clean up unused classes from the imports module

// TODO: Since plasma is now dealing with activity model wallpapers,
//       replace ActivityModel with the KActivities::ActivitiesModel
//       (but keep the name)

ActivitiesExtensionPlugin::ActivitiesExtensionPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

void ActivitiesExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.activities"));

    // Used by applets/activitybar
    qmlRegisterType<KActivities::Imports::ActivityModel>(uri, 0, 1, "ActivityModel");

    qmlRegisterType<KActivities::Imports::ActivityInfo>(uri, 0, 1, "ActivityInfo");
    qmlRegisterType<KActivities::Imports::ResourceInstance>(uri, 0, 1, "ResourceInstance");

    // This one is removed in favor of KActivities::Stats::ResultModel.
    // Subclass it, and make it do what you want.
    // qmlRegisterType<KActivities::Imports::ResourceModel>(uri, 0, 1, "ResourceModel");
}
