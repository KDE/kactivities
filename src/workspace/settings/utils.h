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

#ifndef UTILS_H
#define UTILS_H

#include <QGuiApplication>

inline std::unique_ptr<QQuickView> createView(QWidget *parent)
{
    auto view = new QQuickView();
    view->setColor(QGuiApplication::palette().window().color());
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    auto container = QWidget::createWindowContainer(view, parent);
    container->setFocusPolicy(Qt::TabFocus);

    parent->layout()->addWidget(container);

    return std::unique_ptr<QQuickView>(view);
}

template <typename View>
inline bool setViewSource(View &view, const QString &file)
{
    QString sourceFile = QStringLiteral(KAMD_KCM_DATADIR) + file;

    if (QFile::exists(sourceFile)) {
        view->setSource(sourceFile);

        return true;

    } else {
        return false;
    }

}

#endif // UTILS_H

