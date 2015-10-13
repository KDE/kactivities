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

#ifndef PRIVACY_TAB_H
#define PRIVACY_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

/**
 * PrivacyTab
 */
class PrivacyTab : public QWidget {
    Q_OBJECT
public:
    PrivacyTab(QWidget *parent);
    ~PrivacyTab();

public Q_SLOTS:
    void defaults();
    void load();
    void save();

private Q_SLOTS:
    void forget(int count, const QString &what);
    void forgetLastHour();
    void forgetTwoHours();
    void forgetDay();
    void forgetAll();

    void spinKeepHistoryValueChanged(int value);

Q_SIGNALS:
    void changed();

private:
    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2
    };

    D_PTR;
};

#endif // PRIVACY_TAB_H
