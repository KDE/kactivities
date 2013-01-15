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

#ifndef MODULE_H
#define MODULE_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>
#include <utils/override.h>

/**
 * Module
 */
class Module: public QObject {
    Q_OBJECT

public:
    explicit Module(const QString & name, QObject * parent = nullptr);
    virtual ~Module();

    static QObject * get(const QString & name);
    static const QHash < QString, QObject * > get();

    virtual bool isFeatureOperational(const QStringList & feature) const;
    virtual bool isFeatureEnabled(const QStringList & feature) const;
    virtual void setFeatureEnabled(const QStringList & feature, bool value);
    virtual QStringList listFeatures(const QStringList & feature) const;

protected:
    static void registerModule(const QString & name, QObject * module);

private:
    D_PTR;
};

#endif // MODULE_H

