/*
 *   Copyright (C) 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ActivitiesTab.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>

#include <QQuickView>
#include <QGuiApplication>
#include <QVBoxLayout>

#include "ExtraActivitiesInterface.h"
#include "definitions.h"

#include <utils/d_ptr_implementation.h>

#include "kactivities-features.h"

#include "utils.h"

class ActivitiesTab::Private {
public:
    std::unique_ptr<QQuickView> viewActivities;
    ExtraActivitiesInterface *extraActivitiesInterface;
};

ActivitiesTab::ActivitiesTab(QWidget *parent)
    : QWidget(parent)
    , d()
{
    new QVBoxLayout(this);

    d->extraActivitiesInterface = new ExtraActivitiesInterface(this);

    d->viewActivities = createView(this);
    d->viewActivities->rootContext()->setContextProperty(
        "kactivitiesExtras", d->extraActivitiesInterface);
    d->viewActivities->setSource(
        QStringLiteral(KAMD_INSTALL_PREFIX "/" KAMD_DATA_DIR)
        + "/workspace/settings/qml/activitiesTab/main.qml");
}

ActivitiesTab::~ActivitiesTab()
{
}

void ActivitiesTab::defaults()
{
}

void ActivitiesTab::load()
{
}

void ActivitiesTab::save()
{
}

#include "ActivitiesTab.moc"
