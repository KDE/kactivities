/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef FEATURES_H
#define FEATURES_H

#include <QObject>
#include <QString>

#include <Module.h>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>

/**
 * Features object provides one interface for clients
 * to access other objects' features
 */
class Features: public Module {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Features")

public:
    Features(QObject * parent = nullptr);
    virtual ~Features();

public Q_SLOTS:
    bool IsFeatureOperational(const QString & feature) const;

    bool IsFeatureEnabled(const QString & feature) const;

    void SetFeatureEnabled(const QString & feature, bool value);

    QStringList ListFeatures(const QString & module) const;

private:
    D_PTR;
};

#endif // FEATURES_H
