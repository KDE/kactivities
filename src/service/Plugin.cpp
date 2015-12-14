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

// Self
#include "Plugin.h"

// KDE
#include <ksharedconfig.h>

// Utils
#include <utils/d_ptr_implementation.h>

// Local
#include "Debug.h"


class Plugin::Private {
public:
    Private()
        : config(Q_NULLPTR)
    {
    }

    QString name;
    KSharedConfig::Ptr config;
};

Plugin::Plugin(QObject *parent)
    : Module(QString(), parent)
    , d()
{
}

Plugin::~Plugin()
{
}

KConfigGroup Plugin::config() const
{
    if (d->name.isEmpty()) {
        qWarning() << "The plugin needs a name in order to have a config section";
        return KConfigGroup();
    }

    if (!d->config) {
        d->config = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerd-pluginsrc"));
    }

    return d->config->group(QStringLiteral("Plugin-") + d->name);
}

void Plugin::setName(const QString &name)
{
    Q_ASSERT_X(d->name.isEmpty(), "Plugin::setName", "The name can not be set twice");
    Q_ASSERT_X(!name.isEmpty(), "Plugin::setName", "The name can not be empty");

    qDebug() << "Setting the name of " << (void*)this << " to " << name;
    d->name = name;
}

QString Plugin::name() const
{
    return d->name;
}

bool Plugin::init(QHash<QString, QObject *> &modules)
{
    if (!name().isEmpty()) {
        modules[name()] = this;
    }

    return true;
}

