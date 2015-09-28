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

#include <QDebug>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QQuickView>
#include <QQuickWidget>
#include <QPushButton>
#include <QDialogButtonBox>

#include <KLocalizedString>

#include "utils/d_ptr_implementation.h"
#include "../utils.h"
#include "kactivities-features.h"

class Dialog::Private {
public:
    QVBoxLayout *layout;
    QTabWidget  *tabs;

    QQuickWidget *tabGeneral;
    QQuickWidget *tabOther;

    QQuickWidget *createTab(const QString &title, const QString &file)
    {
        auto view = new QQuickWidget();

        view->setResizeMode(QQuickWidget::SizeRootObjectToView);
        view->setClearColor(QGuiApplication::palette().window().color());

        view->setSource(QUrl::fromLocalFile(
            QStringLiteral(KAMD_INSTALL_PREFIX "/" KAMD_DATA_DIR)
            + "/workspace/settings/qml/activityDialog/" + file));

        tabs->addTab(view, title);

        return view;
    }


};

Dialog::Dialog(QObject *parent)
    : QDialog()
{
    setWindowTitle(i18n("Create a new activity"));
    initUi();
}

Dialog::Dialog(const QString &activityId, QObject *parent)
    : QDialog()
{
    setWindowTitle(i18n("Activity settings"));
    initUi();
}

void Dialog::initUi()
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
}

Dialog::~Dialog()
{
}

void Dialog::showEvent(QShowEvent *event)
{
}

#include "dialog.moc"


