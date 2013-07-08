/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <Application.h>

#include <QDebug>
#include <QDBusConnection>
#include <QThread>

#include <KCrash>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KServiceTypeTrader>
#include <KSharedConfig>
#include <kdbusconnectionpool.h>

#include <Activities.h>
#include <Resources.h>
#include <Features.h>
#include <Plugin.h>

#include <signal.h>
#include <stdlib.h>
#include <memory>

#include <utils/nullptr.h>
#include <utils/override.h>
#include <utils/d_ptr_implementation.h>
#include <utils/val.h>

namespace {
    QList < QThread * > s_moduleThreads;
}

// Runs a QObject inside a QThread

template <typename T>
T * runInQThread()
{
    T * object = new T();

    class Thread: public QThread {
    public:
        Thread(T * ptr = nullptr)
            : QThread(), object(ptr)
        {
        }

        void run() _override
        {
            std::unique_ptr<T> o(object);
            exec();
        }

    private:
        T * object;

    } * thread = new Thread(object);

    s_moduleThreads << thread;

    object->moveToThread(thread);
    thread->start();

    return object;
}

class Application::Private {
public:
    Private()
        : resources  (runInQThread <Resources>  ()),
          activities (runInQThread <Activities> ()),
          features   (runInQThread <Features>   ())
    {
    }

    Resources  * resources;
    Activities * activities;
    Features   * features;

    QList < Plugin * > plugins;

    static Application * s_instance;

};

Application * Application::Private::s_instance = nullptr;

Application::Application()
    : KUniqueApplication(), d()
{
    // TODO: We should move away from any GUI code
    setQuitOnLastWindowClosed(false);

    if (!KDBusConnectionPool::threadConnection().registerService("org.kde.ActivityManager")) {
        exit(0);
    }

    // KAMD is a daemon, if it crashes it is not a problem as
    // long as it restarts properly
    // NOTE: We have a custom crash handler
    KCrash::setFlags(KCrash::AutoRestart);

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

void Application::loadPlugins()
{
    val offers = KServiceTypeTrader::self()->query("ActivityManager/Plugin");

    val config = KSharedConfig::openConfig("activitymanagerrc");
    auto disabledPlugins = config->group("Global").readEntry("disabledPlugins", QStringList());

    val pluginsGroup = config->group("Plugins");
    foreach (const QString & plugin, pluginsGroup.keyList()) {
        if (!pluginsGroup.readEntry(plugin, true))
            disabledPlugins << plugin;
    }

    // Adding overridden plugins into the list of disabled ones

    foreach (val & service, offers) {
        if (!disabledPlugins.contains(service->library())) {
            disabledPlugins.append(
                    service->property("X-ActivityManager-PluginOverrides", QVariant::StringList).toStringList()
                );
        }
    }

    qDebug() << "These are the disabled plugins:" << disabledPlugins;

    // Loading plugins and initializing them
    foreach (val & service, offers) {
        if (disabledPlugins.contains(service->library()) ||
                disabledPlugins.contains(service->property("X-KDE-PluginInfo-Name").toString() + "Enabled")) {
            continue;
        }

        val factory = KPluginLoader(service->library()).factory();

        if (!factory) {
            continue;
        }

        val plugin = factory->create < Plugin > (this);

        if (plugin) {
            qDebug() << "Got the plugin: " << service->library();
            d->plugins << plugin;
        }
    }

    foreach (Plugin * plugin, d->plugins) {
        plugin->init(Module::get());
    }
}

Application::~Application()
{
    foreach (val plugin, d->plugins) {
        delete plugin;
    }

    foreach (val thread, s_moduleThreads) {
        thread->quit();
        thread->wait();

        delete thread;
    }

    Private::s_instance = nullptr;
}

int Application::newInstance()
{
    //We don't want to show the mainWindow()
    return 0;
}

Activities & Application::activities() const
{
    return *d->activities;
}

Resources & Application::resources()  const
{
    return *d->resources;
}

Application * Application::self()
{
    if (!Private::s_instance) {
        Private::s_instance = new Application();
    }

    return Private::s_instance;
}

void Application::quit()
{
    if (Private::s_instance) {
        Private::s_instance->exit();
        delete Private::s_instance;
    }
}


// Leaving object oriented world :)

int main(int argc, char ** argv)
{
    KAboutData about("kactivitymanagerd", nullptr, ki18n("KDE Activity Manager"), "3.0",
            ki18n("KDE Activity Management Service"),
            KAboutData::License_GPL,
            ki18n("(c) 2010, 2011, 2012 Ivan Cukic"), KLocalizedString(),
            "http://www.kde.org/");

    KCmdLineArgs::init(argc, argv, &about);

    return Application::self()->exec();
}

