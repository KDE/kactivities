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

#include "dialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QKeySequence>
#include <QPushButton>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QString>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KGlobalAccel>

#include "kactivities-features.h"
#include "kactivities/info.h"
#include "kactivities/controller.h"
#include "features_interface.h"

#include "common/dbus/common.h"
#include "utils/continue_with.h"
#include "utils/d_ptr_implementation.h"
#include "../utils.h"

class Dialog::Private {
public:
    Private(Dialog *parent)
        : q(parent)
        , activityName("activityName")
        , activityDescription("activityDescription")
        , activityIcon("activityIcon")
        , activityWallpaper("activityWallpaper")
        , activityIsPrivate(true)
        , activityShortcut("activityShortcut")
        , features(new KAMD_DBUS_CLASS_INTERFACE(Features, Features, q))
    {
    }

    Dialog *const q;
    QVBoxLayout *layout;
    QTabWidget  *tabs;

    QQuickWidget *tabGeneral;
    QQuickWidget *tabOther;

    QQuickWidget *createTab(const QString &title, const QString &file)
    {
        auto view = new QQuickWidget();

        view->setResizeMode(QQuickWidget::SizeRootObjectToView);

// TODO: Remove this once we start requiring Qt 5.4
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
        view->quickWindow()->setColor(QGuiApplication::palette().window().color());
#else
        view->setClearColor(QGuiApplication::palette().window().color());
#endif

        view->rootContext()->setContextProperty("dialog", q);
        view->setSource(QUrl::fromLocalFile(
            QStringLiteral(KAMD_INSTALL_PREFIX "/" KAMD_DATA_DIR)
            + "/workspace/settings/qml/activityDialog/" + file));

        tabs->addTab(view, title);

        auto root = view->rootObject();
        Q_ASSERT(root);
        QMetaObject::invokeMethod(root, "load", Qt::DirectConnection);

        // root->setProperty("activityName", "TEST");

        return view;
    }

    void setFocus(QQuickWidget *widget)
    {
        // TODO: does not work...
        widget->setFocus();
        QMetaObject::invokeMethod(widget->rootObject(), "setFocus",
                                  Qt::DirectConnection);
    }

    QString activityId;

    QString activityName;
    QString activityDescription;
    QString activityIcon;
    QString activityWallpaper;
    bool activityIsPrivate;
    QString activityShortcut;

    KActivities::Info *activityInfo;
    KActivities::Controller activities;
    org::kde::ActivityManager::Features *features;
};

Dialog::Dialog(QObject *parent)
    : QDialog()
    , d(this)
{
    setWindowTitle(i18n("Create a new activity"));
    initUi();
}

Dialog::Dialog(const QString &activityId, QObject *parent)
    : QDialog()
    , d(this)
{
    setWindowTitle(i18n("Activity settings"));
    initUi(activityId);

    setActivityId(activityId);

    d->activityInfo = new KActivities::Info(activityId, this);

    setActivityName(d->activityInfo->name());
    setActivityDescription(d->activityInfo->description());
    setActivityIcon(d->activityInfo->icon());

    // finding the key shortcut
    const auto shortcuts = KGlobalAccel::self()->globalShortcut(
        QStringLiteral("ActivityManager"), "switch-to-activity-" + activityId);
    setActivityShortcut(shortcuts.isEmpty() ? QKeySequence() : shortcuts.first());

    // is private?
    auto result = d->features->GetValue(
        "org.kde.ActivityManager.Resources.Scoring/isOTR/" + activityId);

    auto watcher = new QDBusPendingCallWatcher(result, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [&](QDBusPendingCallWatcher *watcher) mutable {
                         QDBusPendingReply<QDBusVariant> reply = *watcher;
                         setActivityIsPrivate(reply.value().variant().toBool());
                     });


}

void Dialog::initUi(const QString &activityId)
{
    resize(600, 500);

    d->layout = new QVBoxLayout(this);

    // Tabs
    d->tabs = new QTabWidget(this);
    d->layout->addWidget(d->tabs);
    d->tabGeneral = d->createTab(i18n("General"), "GeneralTab.qml");
    d->tabOther   = d->createTab(i18n("Other"),   "OtherTab.qml");

    // Buttons
    auto buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->layout->QLayout::addWidget(buttons);

    if (activityId.isEmpty()) {
        buttons->button(QDialogButtonBox::Ok)->setText(i18n("Create"));
    }

    connect(buttons->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &Dialog::save);
    connect(buttons, &QDialogButtonBox::rejected,
            this, &Dialog::close);

    setActivityName(QString());
    setActivityDescription(QString());
    setActivityIcon(QString());
    setActivityIsPrivate(false);

    setActivityShortcut(QKeySequence());
}

Dialog::~Dialog()
{
}

void Dialog::showEvent(QShowEvent *event)
{
    // Setting the focus
    d->setFocus(d->tabGeneral);
}

#define IMPLEMENT_PROPERTY(Scope, Type, PType, PropName)                       \
    Type Dialog::activity##PropName() const                                    \
    {                                                                          \
        auto root = d->tab##Scope->rootObject();                               \
        Q_ASSERT(root);                                                        \
        return root->property("activity" #PropName).value<Type>();             \
    }                                                                          \
    void Dialog::setActivity##PropName(PType value)                            \
    {                                                                          \
        auto root = d->tab##Scope->rootObject();                               \
        Q_ASSERT(root);                                                        \
        root->setProperty("activity" #PropName, value);                        \
    }

IMPLEMENT_PROPERTY(General, QString,      const QString &,      Id)
IMPLEMENT_PROPERTY(General, QString,      const QString &,      Name)
IMPLEMENT_PROPERTY(General, QString,      const QString &,      Description)
IMPLEMENT_PROPERTY(General, QString,      const QString &,      Icon)
IMPLEMENT_PROPERTY(General, QString,      const QString &,      Wallpaper)
IMPLEMENT_PROPERTY(Other,   QKeySequence, const QKeySequence &, Shortcut)
IMPLEMENT_PROPERTY(Other,   bool,         bool,                 IsPrivate)
#undef IMPLEMENT_PROPERTY

void Dialog::save()
{
    if (activityId().isEmpty()) {
        create();

    } else {
        saveChanges(activityId());

    }
}

void Dialog::create()
{
    using namespace kamd::utils;
    continue_with(
        d->activities.addActivity(activityName()),
        [this](const optional_view<QString> &activityId) {
            if (activityId.is_initialized()) {
                saveChanges(activityId.get());
            }
        });
}

void Dialog::saveChanges(const QString &activityId)
{
    d->activities.setActivityName(activityId, activityName());
    d->activities.setActivityDescription(activityId, activityDescription());
    d->activities.setActivityIcon(activityId, activityIcon());

    // setting the key shortcut
    QAction action(Q_NULLPTR);
    action.setProperty("isConfigurationAction", true);
    action.setProperty("componentName", "ActivityManager");
    action.setObjectName("switch-to-activity-" + activityId);
    KGlobalAccel::self()->removeAllShortcuts(&action);
    KGlobalAccel::self()->setGlobalShortcut(&action, activityShortcut());

    // is private?
    d->features->SetValue("org.kde.ActivityManager.Resources.Scoring/isOTR/"
                              + activityId,
                          QDBusVariant(activityIsPrivate()));

    close();
}

#include "dialog.moc"


