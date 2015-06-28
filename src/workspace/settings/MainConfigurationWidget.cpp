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

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KService>
#include <KServiceTypeTrader>
#include <KSharedConfig>

#include "ui_MainConfigurationWidgetBase.h"
#include "BlacklistedApplicationsModel.h"
#include "definitions.h"

#include <utils/d_ptr_implementation.h>

#include "kactivities-features.h"
#include "common/dbus/common.h"

K_PLUGIN_FACTORY(ActivitiesKCMFactory, registerPlugin<MainConfigurationWidget>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase {
public:
    KSharedConfig::Ptr mainConfig;
    KSharedConfig::Ptr pluginConfig;
    BlacklistedApplicationsModel *blacklistedApplicationsModel;

    QObject *viewBlacklistedApplicationsRoot;
    QQuickView *viewBlacklistedApplications;
    KActionCollection *mainActionCollection;
    KActionCollection *activitiesActionCollection;
    KActivities::Consumer activities;

    void createAction(const QString &actionName, const QString &actionText,
                      const QList<QKeySequence> &sequence)
    {
        auto action = mainActionCollection->addAction(actionName);
        action->setProperty("isConfigurationAction", true);
        action->setText(actionText);
        KGlobalAccel::self()->setShortcut(action, sequence);
    }

    Private()
        : viewBlacklistedApplicationsRoot(Q_NULLPTR)
        , viewBlacklistedApplications(Q_NULLPTR)
        , mainActionCollection(Q_NULLPTR)
        , activitiesActionCollection(Q_NULLPTR)
    {
    }
};

void MainConfigurationWidget::activitiesStateChanged(KActivities::Consumer::ServiceStatus status)
{
    if (status == KActivities::Consumer::Running && !d->activitiesActionCollection) {
        d->activitiesActionCollection = new KActionCollection(this, QStringLiteral("ActivityManager"));
        d->activitiesActionCollection->setComponentDisplayName(i18n("Activities"));
        d->activitiesActionCollection->setConfigGlobal(true);

        auto activities = d->activities.activities(KActivities::Info::Running);

        for (const auto &activity: activities) {
            KActivities::Info info(activity);
            auto action = d->activitiesActionCollection->addAction("switch-to-activity-" + activity);
            action->setProperty("isConfigurationAction", true);
            action->setText(i18nc("@action", "Switch to activity \"%1\"", info.name()));
            KGlobalAccel::self()->setShortcut(action, {});
        }

        d->scActivities->addCollection(d->activitiesActionCollection);
    }
}

MainConfigurationWidget::MainConfigurationWidget(QWidget *parent, QVariantList args)
    : KCModule(parent, args)
    , d()
{
    Q_UNUSED(args);

    d->setupUi(this);

    d->mainConfig = KSharedConfig::openConfig("kactivitymanagerdrc");
    d->pluginConfig = KSharedConfig::openConfig("kactivitymanagerd-pluginsrc");

    // Shortcut config. The shortcut belongs to the component "plasmashell"!
    d->mainActionCollection = new KActionCollection(this, QStringLiteral("plasmashell"));
    d->mainActionCollection->setComponentDisplayName(i18n("Activity switching"));
    d->mainActionCollection->setConfigGlobal(true);

    d->createAction("next activity", i18nc("@action", "Walk through activities"),
                    { Qt::META + Qt::Key_Tab });
    d->createAction("previous activity", i18nc("@action", "Walk through activities (Reverse)"),
                    { Qt::META + Qt::SHIFT + Qt::Key_Tab } );

    d->scActivities->setActionTypes(KShortcutsEditor::GlobalAction);
    d->scActivities->addCollection(d->mainActionCollection);

    // Now, the shortcuts for the activities.
    connect(&d->activities, &KActivities::Consumer::serviceStatusChanged,
            this, &MainConfigurationWidget::activitiesStateChanged);
    activitiesStateChanged(d->activities.serviceStatus());

    // Keep history initialization

    d->spinKeepHistory->setRange(0, INT_MAX);
    d->spinKeepHistory->setSpecialValueText(i18nc("unlimited number of months", "forever"));

    connect(d->spinKeepHistory, SIGNAL(valueChanged(int)),
            this, SLOT(spinKeepHistoryValueChanged(int)));
    spinKeepHistoryValueChanged(0);

    // Clear recent history button

    auto menu = new QMenu(this);

    connect(menu->addAction(i18n("Forget the last hour")), &QAction::triggered,
            this, &MainConfigurationWidget::forgetLastHour);
    connect(menu->addAction(i18n("Forget the last two hours")), &QAction::triggered,
            this, &MainConfigurationWidget::forgetTwoHours);
    connect(menu->addAction(i18n("Forget a day")), &QAction::triggered,
            this, &MainConfigurationWidget::forgetDay);
    connect(menu->addAction(i18n("Forget everything")), &QAction::triggered,
            this, &MainConfigurationWidget::forgetAll);

    d->buttonClearRecentHistory->setMenu(menu);

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
    connect(d->blacklistedApplicationsModel, SIGNAL(changed()), this, SLOT(changed()));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->blacklistedApplicationsModel, SLOT(setEnabled(bool)));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->viewBlacklistedApplicationsContainer, SLOT(setEnabled(bool)));

    connect(d->radioRememberSpecificApplications, SIGNAL(toggled(bool)),
            d->checkBlacklistAllNotOnList, SLOT(setEnabled(bool)));
    connect(d->scActivities, &KShortcutsEditor::keyChange,
            this, [this] { changed(); });
    connect(d->checkRememberVirtualDesktop, SIGNAL(toggled(bool)),
            this, SLOT(changed()));

    defaults();

    d->checkBlacklistAllNotOnList->setEnabled(false);
    d->blacklistedApplicationsModel->setEnabled(false);
    d->viewBlacklistedApplicationsContainer->setEnabled(false);
}

MainConfigurationWidget::~MainConfigurationWidget()
{
}

void MainConfigurationWidget::shortcutChanged(const QKeySequence &sequence)
{
    QString actionName = sender() ? sender()->property("shortcutAction").toString() : QString();

    if (actionName.isEmpty()) return;

    auto action = d->mainActionCollection->action(actionName);

    KGlobalAccel::self()->setShortcut(action, { sequence }, KGlobalAccel::NoAutoloading);
    d->mainActionCollection->writeSettings();

    changed();
}

void MainConfigurationWidget::defaults()
{
    d->radioRememberAllApplications->click();
    d->spinKeepHistory->setValue(0);
    d->blacklistedApplicationsModel->defaults();
}

void MainConfigurationWidget::load()
{
    d->blacklistedApplicationsModel->load();

    const auto statisticsConfig = d->pluginConfig->group(SQLITE_PLUGIN_CONFIG_KEY);

    const auto whatToRemember = (WhatToRemember)statisticsConfig.readEntry(
        "what-to-remember", (int)AllApplications);

    d->radioRememberAllApplications->setChecked(whatToRemember == AllApplications);
    d->radioRememberSpecificApplications->setChecked(whatToRemember == SpecificApplications);
    d->radioDontRememberApplications->setChecked(whatToRemember == NoApplications);

    d->spinKeepHistory->setValue(statisticsConfig.readEntry("keep-history-for", 0));
    d->checkBlacklistAllNotOnList->setChecked(
        statisticsConfig.readEntry("blocked-by-default", false));

    auto pluginListConfig = d->mainConfig->group("Plugins");
    d->checkRememberVirtualDesktop->setChecked(
        pluginListConfig.readEntry("org.kde.ActivityManager.VirtualDesktopSwitchEnabled", false));

    // Loading shortcuts
}

void MainConfigurationWidget::save()
{
    d->blacklistedApplicationsModel->save();

    auto statisticsConfig = d->pluginConfig->group(SQLITE_PLUGIN_CONFIG_KEY);

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
    pluginListConfig.writeEntry("org.kde.ActivityManager.VirtualDesktopSwitchEnabled",
                                d->checkRememberVirtualDesktop->isChecked());

    statisticsConfig.sync();
    pluginListConfig.sync();
}

void MainConfigurationWidget::forget(int count, const QString &what)
{
    KAMD_DECL_DBUS_INTERFACE(rankingsservice, Resources/Scoring, ResourcesScoring);

    rankingsservice.asyncCall(
        "DeleteRecentStats", QString(), count, what);
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
