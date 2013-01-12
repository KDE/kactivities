/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <config-features.h>

#include "NepomukPlugin.h"

#include "../../Event.h"

#include "kao.h"

#include <QFileSystemWatcher>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

#include <Nepomuk2/Resource>

#include <KStandardDirs>

#include <utils/nullptr.h>
#include <utils/val.h>

#include "kao.h"

namespace Nepomuk = Nepomuk2;
using namespace KDE::Vocabulary;

NepomukPlugin * NepomukPlugin::s_instance = nullptr;

static inline Nepomuk::Resource activityResource(const QString & id)
{
    return Nepomuk::Resource(id, KAO::Activity());
}

NepomukPlugin::NepomukPlugin(QObject *parent, const QVariantList & args)
    : Plugin(parent),
      m_activities(nullptr)
{
    Q_UNUSED(args)
    s_instance = this;
}

bool NepomukPlugin::init(const QHash < QString, QObject * > & modules)
{
    m_activities = modules["activities"];

    setName("org.kde.ActivityManager.Nepomuk");

    return true;
}

NepomukPlugin * NepomukPlugin::self()
{
    return s_instance;
}

QString NepomukPlugin::currentActivity() const
{
    return Plugin::callOn <QString, Qt::DirectConnection> (m_activities, "CurrentActivity", "QString");
}

void NepomukPlugin::setActivityName(const QString & activity, const QString & name)
{
}

void NepomukPlugin::setActivityIcon(const QString & activity, const QString & icon)
{
}

void NepomukPlugin::setCurrentActivity(const QString & activity)
{
}

void NepomukPlugin::addActivity(const QString & activity)
{
}

void NepomukPlugin::removeActivity(const QString & activity)
{
}


void NepomukPlugin::nepomukServiceStarted()
{
}

void NepomukPlugin::nepomukServiceStopped()
{
}


KAMD_EXPORT_PLUGIN(NepomukPlugin, "activitymanger_plugin_nepomuk")

