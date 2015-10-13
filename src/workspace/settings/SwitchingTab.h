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

#ifndef SWITCHING_TAB_H
#define SWITCHING_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

class QKeySequence;

/**
 * SwitchingTab
 */
class SwitchingTab : public QWidget {
    Q_OBJECT
public:
    SwitchingTab(QWidget *parent);
    ~SwitchingTab();

public Q_SLOTS:
    void defaults();
    void load();
    void save();

private Q_SLOTS:
    void shortcutChanged(const QKeySequence &sequence);

Q_SIGNALS:
    void changed();

private:
    D_PTR;
};

#endif // SWITCHING_TAB_H
