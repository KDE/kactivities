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

#include <Features.h>
#include "featuresadaptor.h"

#include "common.h"
#include <kdbusconnectionpool.h>

#include <utils/d_ptr_implementation.h>
#include <utils/val.h>

class Features::Private {

};

Features::Features(QObject * parent)
    : Module("features", parent), d()
{
    new FeaturesAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
            ACTIVITY_MANAGER_OBJECT_PATH(Features), this);
}

Features::~Features()
{
}

// Features object is just a gateway to the other KAMD modules.
// This is a convenience method to pass the request down to the module

template <typename RetType, typename Function>
static RetType passToModule(const QString & feature, RetType defaultResult, Function f)
{
    val params = feature.split('/');
    val module = Module::get(params.first());

    if (!module) return defaultResult;

    return f(static_cast<Module*>(module), params.mid(1));
}

#define FEATURES_PASS_TO_MODULE(RetType, DefaultResult, What)          \
    passToModule(feature, DefaultResult,                               \
        [=] (Module * module, const QStringList & params) -> RetType { \
            What                                                       \
        });

bool Features::IsFeatureOperational(const QString & feature) const
{
    if (feature.isEmpty()) return false;

    return FEATURES_PASS_TO_MODULE(bool, false,
        return module->isFeatureOperational(params);
    );
}

bool Features::IsFeatureEnabled(const QString & feature) const
{
    if (feature.isEmpty()) return false;

    return FEATURES_PASS_TO_MODULE(bool, false,
        return module->isFeatureEnabled(params);
    );
}

void Features::SetFeatureEnabled(const QString & feature, bool value)
{
    if (feature.isEmpty()) return;

    FEATURES_PASS_TO_MODULE(bool, false,
        module->setFeatureEnabled(params, value);
        return true;
    );
}

QStringList Features::ListFeatures(const QString & feature) const
{
    if (feature.isEmpty()) {
        return Module::get().keys();
    }

    return FEATURES_PASS_TO_MODULE(QStringList, QStringList(),
        return module->listFeatures(params);
    );
}

#undef FEATURES_PASS_TO_MODULE

