/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "activitiescomponentplugin.h"

#include <QtDeclarative/qdeclarative.h>

#include "resourcemodel.h"
#include "activitymodel.h"

#include <QDebug>

#define REGISTER_MODEL(Title, Icon, Type)                           \
    QML_REGISTER_TYPE(Type);                                        \
    AvailableModels::addModel(Title, QIcon::fromTheme(Icon), #Type)


void ActivitiesComponentDataPlugin::registerTypes(const char * uri)
{
    qDebug() << "###########";
    Q_ASSERT(uri == QLatin1String("org.kde.activities.models"));

    qmlRegisterType < KActivities::Models::ResourceModel > (uri, 0, 1, "ResourceModel");
    qmlRegisterType < KActivities::Models::ActivityModel > (uri, 0, 1, "ActivityModel");

}

#include "activitiescomponentplugin.moc"

