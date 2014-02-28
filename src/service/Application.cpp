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

// Self
#include <kactivities-features.h>
#include "Application.h"

// Qt
#include <QDBusConnection>
#include <QThread>
#include <QDir>

// KDE
// #include <KCrash>
// #include <KAboutData>
// #include <KCmdLineArgs>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>
#include <kdbusconnectionpool.h>
#include <kdbusservice.h>

// Boost and utils
#include <boost/range/adaptor/filtered.hpp>
#include <utils/d_ptr_implementation.h>

// System
#include <signal.h>
#include <stdlib.h>
#include <memory>

// Local
#include "Activities.h"
#include "Resources.h"
#include "Features.h"
#include "Plugin.h"
#include "Debug.h"


namespace {
    QList<QThread *> s_moduleThreads;
}

// Runs a QObject inside a QThread

template <typename T>
T *runInQThread()
{
    T *object = new T();

    class Thread : public QThread {
    public:
        Thread(T *ptr = Q_NULLPTR)
            : QThread()
            , object(ptr)
        {
        }

        void run() Q_DECL_OVERRIDE
        {
            std::unique_ptr<T> o(object);
            exec();
        }

    private:
        T *object;

    } *thread = new Thread(object);

    s_moduleThreads << thread;

    object->moveToThread(thread);
    thread->start();

    return object;
}

class Application::Private {
public:
    Private()
        : resources(runInQThread<Resources>())
        , activities(runInQThread<Activities>())
        , features(runInQThread<Features>())
    {
    }

    static inline bool isPluginEnabled(const KConfigGroup &config,
                                const QString &plugin)
    {
        return config.readEntry(plugin, true);
    }

    Resources *resources;
    Activities *activities;
    Features *features;

    QList<Plugin *> plugins;

    static Application *s_instance;
};

Application *Application::Private::s_instance = Q_NULLPTR;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    if (!KDBusConnectionPool::threadConnection().registerService(
             QStringLiteral("org.kde.ActivityManager"))) {
        exit(0);
    }

    // KAMD is a daemon, if it crashes it is not a problem as
    // long as it restarts properly
    // TODO:
    // KCrash::setFlags(KCrash::AutoRestart);

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

void Application::loadPlugins()
{
    using namespace boost::adaptors;
    using namespace std::placeholders;

    // TODO: Return the plugin system
    // TODO: Properly load plugins when KF5::KService becomes more stable

    const QDir pluginsDir(QStringLiteral(KAMD_INSTALL_PREFIX "/" KAMD_PLUGIN_DIR));
    const auto plugins = pluginsDir.entryList(QStringList{ QStringLiteral("activitymanager*.so") }, QDir::Files);
    const auto config = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerdrc"))->group("Plugins");

    const auto availablePlugins = plugins
            | filtered(std::bind(Private::isPluginEnabled, config, _1));

    for (const auto &plugin: availablePlugins) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(plugin));
        qDebug() << pluginsDir.absoluteFilePath(plugin);

        auto pluginInstance = dynamic_cast<Plugin *>(loader.instance());

        if (pluginInstance) {
            pluginInstance->init(Module::get());
            qCDebug(KAMD_LOG_APPLICATION)   << "[   OK   ] loaded:  " << plugin;

        } else {
            qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] loading: " << plugin
                       << loader.errorString();
            // TODO: Show a notification
        }
    }

    // const auto offers = KServiceTypeTrader::self()->query(QStringLiteral("ActivityManager/Plugin"));

    // for (const auto &service: offers) {
    //     if (!disabledPlugins.contains(service->library())) {
    //         disabledPlugins.append(
    //                 service->property("X-ActivityManager-PluginOverrides", QVariant::StringList).toStringList()
    //             );
    //     }
    // }

    // qCDebug(KAMD_APPLICATION) << "These are the disabled plugins:" << disabledPlugins;

    // // Loading plugins and initializing them
    // for (const auto &service: offers) {
    //     if (disabledPlugins.contains(service->library()) ||
    //             disabledPlugins.contains(service->property("X-KDE-PluginInfo-Name").toString() + "Enabled")) {
    //         continue;
    //     }

    //     const auto factory = KPluginLoader(service->library()).factory();

    //     if (!factory) {
    //         continue;
    //     }

    //     const auto plugin = factory->create < Plugin > (this);

    //     if (plugin) {
    //         qCDebug(KAMD_APPLICATION) << "Got the plugin: " << service->library();
    //         d->plugins << plugin;
    //     }
    // }

    // for (Plugin * plugin: d->plugins) {
    //     plugin->init(Module::get());
    // }
}

Application::~Application()
{
    for (const auto plugin: d->plugins) {
        delete plugin;
    }

    for (const auto thread: s_moduleThreads) {
        thread->quit();
        thread->wait();

        delete thread;
    }

    Private::s_instance = Q_NULLPTR;
}

int Application::newInstance()
{
    //We don't want to show the mainWindow()
    return 0;
}

Activities &Application::activities() const
{
    return *d->activities;
}

Resources &Application::resources() const
{
    return *d->resources;
}

void Application::quit()
{
    if (Private::s_instance) {
        Private::s_instance->exit();
        delete Private::s_instance;
    }
}

// Leaving object oriented world :)

int main(int argc, char **argv)
{
    Application application(argc, argv);
    application.setApplicationName(QStringLiteral("ActivityManager"));
    application.setOrganizationDomain(QStringLiteral("kde.org"));

    // KAboutData about("kactivitymanagerd", Q_NULLPTR, ki18n("KDE Activity Manager"), "3.0",
    //         ki18n("KDE Activity Management Service"),
    //         KAboutData::License_GPL,
    //         ki18n("(c) 2010, 2011, 2012 Ivan Cukic"), KLocalizedString(),
    //         "http://www.kde.org/");

    // KCmdLineArgs::init(argc, argv, &about);

    KDBusService service(KDBusService::Unique);

#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
    KAMD_LOG_APPLICATION().setEnabled(QtDebugMsg, true);
    KAMD_LOG_RESOURCES()  .setEnabled(QtDebugMsg, true);
    KAMD_LOG_ACTIVITIES() .setEnabled(QtDebugMsg, true);
    KAMD_LOG_APPLICATION().setEnabled(QtWarningMsg, true);
    KAMD_LOG_RESOURCES()  .setEnabled(QtWarningMsg, true);
    KAMD_LOG_ACTIVITIES() .setEnabled(QtWarningMsg, true);
#endif

    return application.exec();
}
