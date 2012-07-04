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

#ifndef JOBS_JOB_FACTORY_H
#define JOBS_JOB_FACTORY_H

#include "Job.h"
#include <QVariant>

#include <utils/d_ptr.h>

#define DECLARE_JOB_FACTORY(Type, ConstructorParams)    \
    Type(QObject * parent)                              \
        :Job(parent)                                    \
    { init(); }                                         \
    class Factory: public JobFactory {                  \
    public:                                             \
        Factory ConstructorParams ;                     \
        Job * createJob(QObject * parent) {             \
            return new Type(parent);                    \
        }                                               \
    }

#define JOB_FACTORY Factory::Factory

#define JOB_FACTORY_PROPERTY(PropertyName)              \
    setProperty(#PropertyName, QVariant::fromValue(PropertyName))

/**
 * JobFactory
 */
class JobFactory {
public:
    JobFactory();
    virtual ~JobFactory();

    virtual Job * create(QObject * parent);

    void setProperty(const QString & key, const QVariant & value);
    void clearProperty(const QString & key);
    void property(const QString & key) const;

    static JobFactory * wrap(Job * job);

protected:
    virtual Job * createJob(QObject * parent) = 0;

private:
    D_PTR;
};

#endif // JOBS_JOB_FACTORY_H

