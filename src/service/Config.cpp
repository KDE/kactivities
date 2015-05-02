/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <Config.h>

#include <functional>

#include <QStandardPaths>
#include <QDebug>

#include <KDirWatch>

#include <utils/d_ptr_implementation.h>

class Config::Private {
public:
    Private(Config *parent)
        : q(parent)
        , mainConfigFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
              + QStringLiteral("/kactivitymanagerdrc"))
        , pluginConfigFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
              + QStringLiteral("/kactivitymanagerd-pluginsrc"))
    {
        using namespace std::placeholders;

        watcher.addFile(mainConfigFile);
        watcher.addFile(pluginConfigFile);

        QObject::connect(
            &watcher, &KDirWatch::created,
            q, std::bind(&Private::configFileChanged, this, _1));
        QObject::connect(
            &watcher, &KDirWatch::dirty,
            q, std::bind(&Private::configFileChanged, this, _1));
    }

    void configFileChanged(const QString &file)
    {
        if (file == pluginConfigFile) {
            emit q->pluginConfigChanged();
        } else {
            emit q->mainConfigChanged();
        }
    }

    KDirWatch watcher;

private:
    Config * const q;

    const QString mainConfigFile;
    const QString pluginConfigFile;
};

Config::Config(QObject *parent)
    : Module("config", parent)
    , d(this)
{
}

Config::~Config()
{
}


