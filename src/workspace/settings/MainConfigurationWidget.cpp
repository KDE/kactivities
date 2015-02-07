/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDBusPendingCall>

#include <QQuickView>

#include <KLocalizedString>
#include <KAboutData>
#include <KPluginSelector>
#include <KPluginInfo>
#include <KService>
#include <KServiceTypeTrader>

#include "ui_MainConfigurationWidgetBase.h"
#include "BlacklistedApplicationsModel.h"

#include <utils/d_ptr_implementation.h>

#include "kactivities-features.h"
#include "common/dbus/common.h"

K_PLUGIN_FACTORY(ActivitiesKCMFactory, registerPlugin<MainConfigurationWidget>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase {
public:
    Ui::MainConfigurationWidgetBase ui;

    KSharedConfig::Ptr mainConfig;
    KSharedConfig::Ptr pluginConfig;
    KPluginSelector *pluginSelector;
    BlacklistedApplicationsModel *blacklistedApplicationsModel;

    QObject *viewBlacklistedApplicationsRoot;
    QQuickView *viewBlacklistedApplications;
};

MainConfigurationWidget::MainConfigurationWidget(QWidget *parent, QVariantList args)
    : KCModule(parent, args)
    , d()
{
    Q_UNUSED(args)

    d->setupUi(this);

    // Plugin selector initialization

    const auto offers = KServiceTypeTrader::self()->query("ActivityManager/Plugin");
    const auto plugins = KPluginInfo::fromServices(offers);

    d->mainConfig = KSharedConfig::openConfig("kactivitymanagerdrc");
    d->pluginConfig = KSharedConfig::openConfig("kactivitymanagerd-pluginsrc");

    // Loading the plugin selector
    d->pluginSelector = new KPluginSelector(this);
    d->pluginSelector->addPlugins(plugins, KPluginSelector::ReadConfigFile,
                                  i18n("Available Features"), QString(),
                                  d->mainConfig);
    d->tabWidget->addTab(d->pluginSelector, i18n("Plugins"));

    // Keep history initialization

    d->spinKeepHistory->setRange(0, INT_MAX);
    d->spinKeepHistory->setSpecialValueText(i18nc("unlimited number of months", "forever"));

    // We don't have KSpingBox anymore, lets keep it alive :)
    connect(d->spinKeepHistory, SIGNAL(valueChanged(int)),
            this, SLOT(spinKeepHistoryValueChanged(int)));
    spinKeepHistoryValueChanged(0);

    // Clear recent history button

    auto menu = new QMenu(this);

    connect(
        menu->addAction(i18n("Forget the last hour")), SIGNAL(triggered()),
        this, SLOT(forgetLastHour()));
    connect(
        menu->addAction(i18n("Forget the last two hours")), SIGNAL(triggered()),
        this, SLOT(forgetTwoHours()));
    connect(
        menu->addAction(i18n("Forget a day")), SIGNAL(triggered()),
        this, SLOT(forgetDay()));
    connect(
        menu->addAction(i18n("Forget everything")), SIGNAL(triggered()),
        this, SLOT(forgetAll()));

    d->buttonClearRecentHistory->setMenu(menu);

    // Activities must run! :)

    d->checkEnableActivities->setVisible(false);

    // Blacklist applications

    d->blacklistedApplicationsModel = new BlacklistedApplicationsModel(this);

    auto layout = new QGridLayout(d->viewBlacklistedApplicationsContainer);

    d->viewBlacklistedApplications = new QQuickView();
    d->viewBlacklistedApplications->setColor(
        QGuiApplication::palette().window().color());

    QWidget *container = QWidget::createWindowContainer(
        d->viewBlacklistedApplications,
        d->viewBlacklistedApplicationsContainer);


    container->setFocusPolicy(Qt::TabFocus);
    d->viewBlacklistedApplications->rootContext()->setContextProperty(
        "applicationModel", d->blacklistedApplicationsModel);
    d->viewBlacklistedApplications->setSource(
        QStringLiteral(KAMD_INSTALL_PREFIX "/" KAMD_DATA_DIR)
        + "/workspace/settings/BlacklistApplicationView.qml");

    layout->addWidget(container);

    // React to changes

    connect(d->radioRememberAllApplications, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->radioDontRememberApplications, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(d->spinKeepHistory, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(d->pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()));
    connect(d->blacklistedApplicationsModel, SIGNAL(changed()), this, SLOT(changed()));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->blacklistedApplicationsModel, SLOT(setEnabled(bool)));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->viewBlacklistedApplicationsContainer, SLOT(setEnabled(bool)));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->checkBlacklistAllNotOnList, SLOT(setEnabled(bool)));

    defaults();

    d->checkBlacklistAllNotOnList->setEnabled(false);
    d->blacklistedApplicationsModel->setEnabled(false);
    d->viewBlacklistedApplicationsContainer->setEnabled(false);
}

MainConfigurationWidget::~MainConfigurationWidget()
{
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

    const auto statisticsConfig = d->pluginConfig->group(
        "Plugin-org.kde.ActivityManager.ResourceScoring");

    const auto whatToRemember = (WhatToRemember)statisticsConfig.readEntry(
        "what-to-remember", (int)AllApplications);

    d->radioRememberAllApplications->setChecked(whatToRemember == AllApplications);
    d->radioRememberSpecificApplications->setChecked(whatToRemember == SpecificApplications);
    d->radioDontRememberApplications->setChecked(whatToRemember == NoApplications);

    d->spinKeepHistory->setValue(statisticsConfig.readEntry("keep-history-for", 0));
    d->checkBlacklistAllNotOnList->setChecked(
        statisticsConfig.readEntry("blocked-by-default", false));
}

void MainConfigurationWidget::save()
{
    d->pluginSelector->save();
    d->blacklistedApplicationsModel->save();

    auto statisticsConfig = d->pluginConfig->group(
        "Plugin-org.kde.ActivityManager.ResourceScoring");

    const auto whatToRemember =
        d->radioRememberSpecificApplications->isChecked() ? SpecificApplications :
        d->radioDontRememberApplications->isChecked()     ? NoApplications :
        /* otherwise */                                     AllApplications;

    statisticsConfig.writeEntry("what-to-remember", (int)whatToRemember);
    statisticsConfig.writeEntry("keep-history-for", d->spinKeepHistory->value());
    statisticsConfig.writeEntry("blocked-by-default", d->checkBlacklistAllNotOnList->isChecked());

    auto pluginListConfig = d->mainConfig->group("Plugins");

    pluginListConfig.writeEntry("org.kde.ActivityManager.ResourceScoringEnabled",
                                whatToRemember != NoApplications);

    statisticsConfig.sync();
    pluginListConfig.sync();
}

void MainConfigurationWidget::forget(int count, const QString &what)
{
    KAMD_DECL_DBUS_INTERFACE(rankingsservice, Resources/Scoring, ResourcesScoring);

    rankingsservice.asyncCall(
        "deleteRecentStats", QString(), count, what);
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

void MainConfigurationWidget::spinKeepHistoryValueChanged(int value)
{
    static auto months = ki18ncp("unit of time. months to keep the history", " month", " months");

    if (value) {
        d->spinKeepHistory->setPrefix(i18nc("for in 'keep history for 5 months'", "for "));
        d->spinKeepHistory->setSuffix(months.subs(value).toString());
    }
}

#include "MainConfigurationWidget.moc"
