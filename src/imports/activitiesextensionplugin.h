/*
    SPDX-FileCopyrightText: 2011, 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KACTIVITIES_ACTIVITIES_EXTENSION_PLUGIN_H
#define KACTIVITIES_ACTIVITIES_EXTENSION_PLUGIN_H

#include <QQmlExtensionPlugin>

class ActivitiesExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.activities")

public:
    explicit ActivitiesExtensionPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // KACTIVITIES_ACTIVITIES_EXTENSION_PLUGIN_H
