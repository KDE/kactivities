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

#include "Abstract.h"
#include "Abstract_p.h"

#include <QDebug>

#include <utils/d_ptr_implementation.h>

namespace Jobs {
namespace Schedulers {

Abstract::Private::Private(Abstract * parent)
    : q(parent)
{
}

void Abstract::Private::jobFinished(KJob * job)
{
    qDebug() << "Job has finished with this result" << job->error();
    q->jobFinished(job->error());
}

bool Abstract::startJob(int index)
{
    d->lastJobStarted = index;

    // If the index is not valid
    if (index < 0 || d->jobs.size() <= index) {
        returnResult(0);
        return false;
    }

    JobFactory * factory = d->jobs[index];

    // If the job factory is null, exit
    if (!factory) {
        returnResult(0);
        return false;
    }

    // Starting the job
    KJob * job = d->jobs[index]->create(this);

    d->connect(job, SIGNAL(finished(KJob *)),
            SLOT(jobFinished(KJob *)));

    job->start();

    return true;
}

Abstract::Abstract(QObject * parent)
    : Job(parent), d(this)
{
    d->lastJobStarted = -1;

    if (!parent) {
        connect(
            this, SIGNAL(finished(KJob *)),
            this, SLOT(deleteLater()),
            Qt::QueuedConnection
        );
    }
}

Abstract::~Abstract()
{
    qDeleteAll(d->jobs);
}

void Abstract::addJob(Job * other)
{
    d->jobs << JobFactory::wrap(other);
}

void Abstract::addJob(JobFactory * other)
{
    d->jobs << other;
}

void Abstract::start()
{
    if (d->jobs.size() == 0) {
        returnResult(0);
        return;
    }

    startJob(0);
}

int Abstract::lastJobStarted() const
{
    return d->lastJobStarted;
}

int Abstract::jobCount() const
{
    return d->jobs.size();
}

bool Abstract::hasJob(int index) const
{
    return (index >= 0 && index < d->jobs.size() && d->jobs[index] != nullptr);
}

void Abstract::returnResult(int result)
{
    qDebug() << "Returning" << result;
    setError(result);
    emitResult();
}


} // namespace Schedulers
} // namespace Jobs
