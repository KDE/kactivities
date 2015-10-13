/*
 *   Copyright (C) 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "SwitchingTab.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KSharedConfig>

#include "ui_SwitchingTabBase.h"

#include <utils/d_ptr_implementation.h>
#include <KActivities/Consumer>

class SwitchingTab::Private : public Ui::SwitchingTabBase {
public:
    KSharedConfig::Ptr mainConfig;

    KActionCollection *mainActionCollection;
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
        : mainActionCollection(Q_NULLPTR)
    {
    }
};

SwitchingTab::SwitchingTab(QWidget *parent)
    : QWidget(parent)
    , d()
{
    d->setupUi(this);

    d->mainConfig = KSharedConfig::openConfig("kactivitymanagerdrc");

    // Shortcut config. The shortcut belongs to the component "plasmashell"!
    d->mainActionCollection = new KActionCollection(this, QStringLiteral("plasmashell"));
    d->mainActionCollection->setComponentDisplayName(i18n("Activity switching"));
    d->mainActionCollection->setConfigGlobal(true);

    d->createAction("next activity",
                    i18nc("@action", "Walk through activities"),
                    { Qt::META + Qt::Key_Tab });
    d->createAction("previous activity",
                    i18nc("@action", "Walk through activities (Reverse)"),
                    { Qt::META + Qt::SHIFT + Qt::Key_Tab } );

    d->scActivities->setActionTypes(KShortcutsEditor::GlobalAction);
    d->scActivities->addCollection(d->mainActionCollection);

    connect(d->scActivities, &KShortcutsEditor::keyChange,
            this, [this] { changed(); });
    connect(d->checkRememberVirtualDesktop, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));

    defaults();
}

SwitchingTab::~SwitchingTab()
{
}

void SwitchingTab::shortcutChanged(const QKeySequence &sequence)
{
    QString actionName = sender()
                             ? sender()->property("shortcutAction").toString()
                             : QString();

    if (actionName.isEmpty()) return;

    auto action = d->mainActionCollection->action(actionName);

    KGlobalAccel::self()->setShortcut(action, { sequence },
                                      KGlobalAccel::NoAutoloading);
    d->mainActionCollection->writeSettings();

    emit changed();
}

void SwitchingTab::defaults()
{
    d->checkRememberVirtualDesktop->setChecked(false);
}

void SwitchingTab::load()
{
    auto pluginListConfig = d->mainConfig->group("Plugins");

    d->checkRememberVirtualDesktop->setChecked(pluginListConfig.readEntry(
        "org.kde.ActivityManager.VirtualDesktopSwitchEnabled", false));
}

void SwitchingTab::save()
{
    auto pluginListConfig = d->mainConfig->group("Plugins");

    pluginListConfig.writeEntry(
        "org.kde.ActivityManager.VirtualDesktopSwitchEnabled",
        d->checkRememberVirtualDesktop->isChecked());

    pluginListConfig.sync();
}

#include "SwitchingTab.moc"
