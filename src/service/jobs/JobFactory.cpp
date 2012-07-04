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

#include "JobFactory.h"

#include <QString>
#include <QVariant>
#include <QHash>
#include <QDebug>

#include <utils/d_ptr_implementation.h>

class JobFactory::Private {
public:
    QHash < QString, QVariant > properties;
};

JobFactory::JobFactory()
    : d()
{
}

JobFactory::~JobFactory()
{
}

void JobFactory::setProperty(const QString & key, const QVariant & value)
{
    d->properties[key] = value;
}

void JobFactory::clearProperty(const QString & key)
{
    d->properties.remove(key);
}

void JobFactory::property(const QString & key) const
{
    d->properties[key];
}

Job * JobFactory::create(QObject * parent)
{
    Job * result = createJob(parent);

    QHashIterator < QString, QVariant > i(d->properties);
    while (i.hasNext()) {
        i.next();
        result->setProperty(i.key().toAscii(), i.value());
    }

    return result;
}

class JobFactoryWrapper: public JobFactory {
public:
    JobFactoryWrapper(Job * job)
        : m_job(job)
    {
    }

protected:
    virtual Job * createJob(QObject * parent) _override
    {
        m_job->setParent(parent);

        return m_job;
    }

private:
    Job * m_job;
};

JobFactory * JobFactory::wrap(Job * job)
{
    return new JobFactoryWrapper(job);
}

// class JobFactory


