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

#include "MainConfigurationWidget.h"

#include <QDebug>
#include <QMenu>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDBusInterface>
#include <QDBusPendingCall>

#include <QGraphicsObject>

#include <KLocalizedString>
#include <KAboutData>
#include <KPluginSelector>
#include <KPluginInfo>
#include <KService>
#include <KServiceTypeTrader>
#include <kdeclarative.h>

#include <Plasma/PackageStructure>

#include "ui_MainConfigurationWidgetBase.h"
#include "BlacklistedApplicationsModel.h"

#include <utils/d_ptr_implementation.h>
#include <utils/val.h>

K_PLUGIN_FACTORY ( ActivitiesKCMFactory, registerPlugin<MainConfigurationWidget>(); )
K_EXPORT_PLUGIN  ( ActivitiesKCMFactory("kcm_activities","kcm_activities") )

class MainConfigurationWidget::Private: public Ui::MainConfigurationWidgetBase {
public:
    Ui::MainConfigurationWidgetBase ui;

    KSharedConfig::Ptr mainConfig;
    KSharedConfig::Ptr pluginConfig;
    KPluginSelector * pluginSelector;
    BlacklistedApplicationsModel * blacklistedApplicationsModel;
    KDeclarative kdeclarative;
    Plasma::PackageStructure::Ptr structure;
    QGraphicsObject * viewBlacklistedApplicationsRoot;
};

MainConfigurationWidget::MainConfigurationWidget(QWidget * parent, QVariantList args)
    : KCModule( ActivitiesKCMFactory::componentData(), parent ), d()
{
    Q_UNUSED(args)

    val about = new KAboutData(
            "kio_activities", 0, ki18n("Activities"),
            KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
            ki18n("(c) 2012 Ivan Cukic")
        );

    setAboutData(about);

    d->setupUi(this);

    // Plugin selector initialization

    val offers = KServiceTypeTrader::self()->query("ActivityManager/Plugin");
    val plugins = KPluginInfo::fromServices(offers);

    d->mainConfig   = KSharedConfig::openConfig("activitymanagerrc");
    d->pluginConfig = KSharedConfig::openConfig("activitymanager-pluginsrc");

    d->pluginSelector = new KPluginSelector(this);
    d->pluginSelector->addPlugins(
            plugins,
            KPluginSelector::ReadConfigFile,
            i18n("Available Features"),
            QString(),
            d->mainConfig
        );
    d->tabWidget->addTab(d->pluginSelector, i18n("Plugins"));

    // Keep history initialization

    d->spinKeepHistory->setRange(0, INT_MAX);

    d->spinKeepHistory->setSuffix(ki18ncp("unit of time. months to keep the history",
                " month", " months"));
    d->spinKeepHistory->setPrefix(i18nc("for in 'keep history for 5 months'", "for "));
    d->spinKeepHistory->setSpecialValueText(i18nc("unlimited number of months", "forever"));

    // Clear recent history button

    auto menu = new QMenu(this);

    connect(
            menu->addAction(i18n("Forget the last hour")), SIGNAL(triggered()),
            this, SLOT(forgetLastHour())
        );
    connect(
            menu->addAction(i18n("Forget the last two hours")), SIGNAL(triggered()),
            this, SLOT(forgetTwoHours())
        );
    connect(
            menu->addAction(i18n("Forget a day")), SIGNAL(triggered()),
            this, SLOT(forgetDay())
        );
    connect(
            menu->addAction(i18n("Forget everything")), SIGNAL(triggered()),
            this, SLOT(forgetAll())
        );

    d->buttonClearRecentHistory->setMenu(menu);

    // Activities must run! :)

    d->checkEnableActivities->setVisible(false);

    // Blacklist applications
    d->blacklistedApplicationsModel = new BlacklistedApplicationsModel(this);

    QGraphicsScene * scene = new QGraphicsScene(this);
    d->viewBlacklistedApplications->setScene(scene);
    QDeclarativeEngine * engine = new QDeclarativeEngine(this);

    d->kdeclarative.setDeclarativeEngine(engine);
    d->kdeclarative.initialize();
    d->kdeclarative.setupBindings();
    d->structure = Plasma::PackageStructure::load("Plasma/Generic");

    engine->rootContext()->setContextProperty("applicationModel", d->blacklistedApplicationsModel);
    QDeclarativeComponent component(engine, QUrl(QString(KAMD_DATA_DIR) + "workspace/settings/BlacklistApplicationView.qml"));
    qDebug() << "Errors: " << component.errors();
    d->viewBlacklistedApplicationsRoot = qobject_cast<QGraphicsObject *>(component.create());
    d->viewBlacklistedApplicationsRoot->setProperty("width", d->viewBlacklistedApplications->width());
    scene->addItem(d->viewBlacklistedApplicationsRoot);

    d->viewBlacklistedApplications->installEventFilter(this);

    // React to changes

    connect(d->checkEnableActivities, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->radioRememberAllApplications, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->radioDontRememberApplications, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->spinKeepHistory, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(d->pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()));
    connect(d->blacklistedApplicationsModel, SIGNAL(changed()), this, SLOT(changed()));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->blacklistedApplicationsModel, SLOT(setEnabled(bool)));
    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->viewBlacklistedApplications, SLOT(setEnabled(bool)));
    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->checkBlacklistAllNotOnList, SLOT(setEnabled(bool)));

    defaults();

    d->checkBlacklistAllNotOnList->setEnabled(false);
    d->blacklistedApplicationsModel->setEnabled(false);
    d->viewBlacklistedApplications->setEnabled(false);
}

void MainConfigurationWidget::updateLayout()
{
    d->viewBlacklistedApplicationsRoot->setProperty("width",
            d->viewBlacklistedApplications->width() - 32
        );
    d->viewBlacklistedApplicationsRoot->setProperty("minimumHeight",
            d->viewBlacklistedApplications->height() - 32
        );
}

bool MainConfigurationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->viewBlacklistedApplications) {
        if (event->type() == QEvent::Resize) {
            updateLayout();
        }
    }

    return false;
}

void MainConfigurationWidget::defaults()
{
    d->checkEnableActivities->setChecked(true);
    d->radioRememberAllApplications->click();
    d->spinKeepHistory->setValue(0);
    d->pluginSelector->defaults();
    d->blacklistedApplicationsModel->defaults();
}

void MainConfigurationWidget::load()
{
    d->pluginSelector->load();
    d->blacklistedApplicationsModel->load();

    val statisticsConfig = d->pluginConfig->group("Plugin-org.kde.kactivitymanager.resourcescoring");

    val whatToRemember = (WhatToRemember) statisticsConfig.readEntry("what-to-remember", (int)AllApplications);
    d->radioRememberAllApplications->setChecked(whatToRemember == AllApplications);
    d->radioRememberSpecificApplications->setChecked(whatToRemember == SpecificApplications);
    d->radioDontRememberApplications->setChecked(whatToRemember == NoApplications);

    d->spinKeepHistory->setValue(statisticsConfig.readEntry("keep-history-for", 0));
    d->checkBlacklistAllNotOnList->setChecked(statisticsConfig.readEntry("blocked-by-default", false));
}

void MainConfigurationWidget::save()
{
    d->pluginSelector->save();
    d->blacklistedApplicationsModel->save();

    auto statisticsConfig = d->pluginConfig->group("Plugin-org.kde.kactivitymanager.resourcescoring");

    WhatToRemember whatToRemember = AllApplications;

    if (d->radioRememberSpecificApplications->isChecked()) {
        whatToRemember = SpecificApplications;
    } else if (d->radioDontRememberApplications->isChecked()) {
        whatToRemember = NoApplications;
    }

    statisticsConfig.writeEntry("what-to-remember", (int)whatToRemember);
    statisticsConfig.writeEntry("keep-history-for", d->spinKeepHistory->value());
    statisticsConfig.writeEntry("blocked-by-default", d->checkBlacklistAllNotOnList->isChecked());

    auto pluginListConfig = d->mainConfig->group("Plugins");

    pluginListConfig.writeEntry("org.kde.kactivitymanager.resourcescoringEnabled",
            whatToRemember != NoApplications);

    statisticsConfig.sync();
    pluginListConfig.sync();
}

void MainConfigurationWidget::forget(int count, const QString & what)
{
    QDBusInterface rankingsservice(
            "org.kde.ActivityManager",
            "/ActivityManager/Resources/Scoring",
            "org.kde.ActivityManager.Resources.Scoring"
        );

    rankingsservice.asyncCall(
            "deleteRecentStats", QString(), count, what
        );
}

void MainConfigurationWidget::forgetLastHour()
{
    forget(1, "h");
}

void MainConfigurationWidget::forgetTwoHours()
{
    forget(2, "h");
}

void MainConfigurationWidget::forgetDay()
{
    forget(1, "d");
}

void MainConfigurationWidget::forgetAll()
{
    forget(0, "everything");
}

#include "MainConfigurationWidget.moc"

