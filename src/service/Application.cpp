/*
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <QProcess>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>

// KDE
// #include <KCrash>
// #include <KAboutData>
// #include <KCmdLineArgs>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>
#include <kdbusconnectionpool.h>
#include <kdbusservice.h>
#include <kplugintrader.h>

// Boost and utils
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <utils/d_ptr_implementation.h>

// System
#include <signal.h>
#include <stdlib.h>
#include <memory>
#include <functional>

// Local
#include "Activities.h"
#include "Resources.h"
#include "Features.h"
#include "Plugin.h"
#include "Debug.h"

#define KAMD_DBUS_SERVICE_NAME QStringLiteral("org.kde.ActivityManager")


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
    {
    }

    static inline bool isPluginEnabled(const KConfigGroup &config,
                                QExplicitlySharedDataPointer<KService> plugin)
                                // const QExplicitlySharedDataPointer<KService> &plugin)
    {
        const auto pluginName
            = plugin->property("X-KDE-PluginInfo-Name",
                               QVariant::String).toString();
        // qDebug() << "Plugin Name is " << pluginName;

        if (pluginName == "org.kde.ActivityManager.ResourceScoring") {
            // SQLite plugin is necessary for the proper workspace behaviour
            return true;

        } else {
            return config.readEntry(
                pluginName + "Enabled",
                plugin->property("X-KDE-PluginInfo-EnabledByDefault",
                                 QVariant::Bool).toBool());
        }
    }

    bool loadPlugin(KService::Ptr offer);

    Resources *resources;
    Activities *activities;
    Features *features;

    QStringList pluginIds;

    static Application *s_instance;
};

Application *Application::Private::s_instance = Q_NULLPTR;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

void Application::init()
{
    if (!KDBusConnectionPool::threadConnection().registerService(
            KAMD_DBUS_SERVICE_NAME)) {
        exit(EXIT_SUCCESS);
    }

    // KAMD is a daemon, if it crashes it is not a problem as
    // long as it restarts properly
    // TODO:
    // KCrash::setFlags(KCrash::AutoRestart);
    d->resources  = runInQThread<Resources>();
    d->activities = runInQThread<Activities>();
    d->features   = runInQThread<Features>();

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);

    QDBusConnection::sessionBus().registerObject("/ActivityManager", this,
            QDBusConnection::ExportAllSlots);
}

bool Application::Private::loadPlugin(KService::Ptr offer)
{
    if (!offer) {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] plugin offer not valid";
        return false;
    }

    if (pluginIds.contains(offer->storageId())) {
        qCDebug(KAMD_LOG_APPLICATION)   << "[   OK   ] already loaded:  " << offer->name();
        return true;
    }

    QString error;
    auto pluginInstance = dynamic_cast<Plugin*>(
            offer->createInstance<QObject>(Q_NULLPTR, {}, &error)
        );

    if (pluginInstance) {
        pluginInstance->init(Module::get());

        pluginIds << offer->storageId();

        qCDebug(KAMD_LOG_APPLICATION)   << "[   OK   ] loaded:  " << offer->name();
        return true;

    } else {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] loading: " << offer->name() << error;
        // TODO: Show a notification
        return false;
    }
}

void Application::loadPlugins()
{
    using namespace boost::adaptors;
    using namespace std::placeholders;

    const auto pluginsDir(QLatin1String(KAMD_PLUGIN_DIR));
    QCoreApplication::addLibraryPath(pluginsDir);

    const auto config
        = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerdrc"))
              ->group("Plugins");
    const auto allOffers
        = KServiceTypeTrader::self()->query("ActivityManager/Plugin");

    const auto filteredOffers = allOffers
        | filtered(std::bind(Private::isPluginEnabled, config, _1));

    for (const auto &offer : filteredOffers) {
        d->loadPlugin(offer);
    }
}

bool Application::loadPlugin(const QString &plugin)
{
    auto offer = KService::serviceByStorageId(plugin);

    if (!offer) {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] not found: " << plugin;
        return false;
    }

    return d->loadPlugin(offer);
}

Application::~Application()
{
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

// void Application::quit()
// {
//     if (Private::s_instance) {
//         Private::s_instance->exit();
//         delete Private::s_instance;
//     }
// }

void Application::quit()
{
    QApplication::quit();
}

#include "../lib/core/version.h"
QString Application::serviceVersion() const
{
    return KACTIVITIES_VERSION_STRING;
}

// Leaving object oriented world :)

namespace  {
    template <typename Return>
    Return callOnRunningService(const QString &method)
    {
        static QDBusInterface remote(KAMD_DBUS_SERVICE_NAME, "/ActivityManager",
                                     "org.kde.ActivityManager.Application");
        QDBusReply<Return> reply = remote.call(method);

        return (Return)reply;
    }

    QString runningServiceVersion()
    {
        return callOnRunningService<QString>("serviceVersion");
    }

    bool isServiceRunning()
    {
        return QDBusConnection::sessionBus().interface()->isServiceRegistered(
                KAMD_DBUS_SERVICE_NAME);
    }
}

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

    const auto arguments = application.arguments();

    if (arguments.size() == 0) {
        exit(EXIT_FAILURE);

    } else if (arguments.size() != 1 && (arguments.size() != 2 || arguments[1] == "--help")) {

        QTextStream(stdout)
            << "start\tStarts the service\n"
            << "stop\tStops the server\n"
            << "status\tPrints basic server information\n"
            << "start-daemon\tStarts the service without forking (use with caution)\n"
            << "--help\tThis help message\n";

        exit(EXIT_SUCCESS);

    } else if (arguments.size() == 1 || arguments[1] == "start") {

        // Checking whether the service is already running
        if (isServiceRunning()) {
            QTextStream(stdout) << "Already running\n";
            exit(EXIT_SUCCESS);
        }

        // Creating the watcher, but not on the wall

        QDBusServiceWatcher watcher(QStringLiteral("org.kde.ActivityManager"),
                QDBusConnection::sessionBus());

        QObject::connect(&watcher, &QDBusServiceWatcher::serviceRegistered,
            [] (const QString &service) {
                if (service != KAMD_DBUS_SERVICE_NAME) {
                    return;
                }

                QTextStream(stdout)
                    << "Service started, version: " << runningServiceVersion()
                    << "\n";

                exit(EXIT_SUCCESS);
            });

        // Starting the dameon

        QProcess::startDetached(
                KAMD_INSTALL_PREFIX "/bin/kactivitymanagerd",
                QStringList{"start-daemon"}
            );

        return application.exec();

    } else if (arguments[1] == "stop") {
        if (!isServiceRunning()) {
            QTextStream(stdout) << "Service not running\n";
            exit(EXIT_SUCCESS);
        }

        callOnRunningService<void>("quit");

        QTextStream(stdout) << "Service stopped\n";

        return EXIT_SUCCESS;

    } else if (arguments[1] == "status") {

        // Checking whether the service is already running
        if (isServiceRunning()) {
            QTextStream(stdout) << "The service is running, version: "
                                << runningServiceVersion() << "\n";
        } else {
            QTextStream(stdout) << "The service is not running\n";

        }

        return EXIT_SUCCESS;

    } else if (arguments[1] == "start-daemon") {
        // Really starting the activity manager

        KDBusService service(KDBusService::Unique);

        #if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
        KAMD_LOG_APPLICATION().setEnabled(QtDebugMsg, true);
        KAMD_LOG_RESOURCES()  .setEnabled(QtDebugMsg, true);
        KAMD_LOG_ACTIVITIES() .setEnabled(QtDebugMsg, true);
        KAMD_LOG_APPLICATION().setEnabled(QtWarningMsg, true);
        KAMD_LOG_RESOURCES()  .setEnabled(QtWarningMsg, true);
        KAMD_LOG_ACTIVITIES() .setEnabled(QtWarningMsg, true);
        #endif

        application.init();

        return application.exec();

    } else {
        QTextStream(stdout) << "Unrecognized command: " << arguments[1] << '\n';

        return EXIT_FAILURE;
    }
}
