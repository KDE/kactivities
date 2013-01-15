/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "Module.h"

#include <QHash>
#include <QString>
#include <QObject>
#include <QDebug>

#include <utils/d_ptr_implementation.h>

class Module::Private {
public:
    static QHash < QString, QObject * > s_modules;

};

QHash < QString, QObject * > Module::Private::s_modules;

Module::Module(const QString & name, QObject * parent)
    : QObject(parent), d()
{
    registerModule(name, this);
}

void Module::registerModule(const QString & name, QObject * module) {
    if (!name.isEmpty()) {
        Private::s_modules[name] = module;
        qDebug() << "Module " << name << "is registered";
    }
}

Module::~Module()
{
}

QObject * Module::get(const QString & name)
{
    Q_ASSERT(!name.isEmpty());

    if (Private::s_modules.contains(name)) {
        qDebug() << "Returning a valid module object for:" << name;
        return Private::s_modules[name];
    }

    qDebug() << "The requested module doesn't exist:" << name;
    return nullptr;
}

const QHash < QString, QObject * > Module::get()
{
    return Private::s_modules;
}

bool Module::isFeatureEnabled(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;
}

bool Module::isFeatureOperational(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;
}

void Module::setFeatureEnabled(const QStringList & feature, bool value)
{
    Q_UNUSED(feature)
    Q_UNUSED(value)
}

QStringList Module::listFeatures(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return QStringList();
}

