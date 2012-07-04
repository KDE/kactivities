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

#ifndef JOBS_JOB_H
#define JOBS_JOB_H

#include <KJob>

#include <utils/override.h>
#include <utils/nullptr.h>
#include <utils/d_ptr.h>

/**
 * Job
 */
class Job: public KJob {
    Q_OBJECT

public:
    Job(QObject * parent = nullptr);
    virtual ~Job();

    virtual void init();

    static QObject * global();

private:
    D_PTR;
};

#endif // JOBS_JOB_H

