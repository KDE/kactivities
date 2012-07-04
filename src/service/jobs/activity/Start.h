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

#ifndef JOBS_ACTIVITY_START_H
#define JOBS_ACTIVITY_START_H

#include "../Job.h"
#include "../JobFactory.h"

namespace Jobs {
namespace Activity {

/**
 * Change
 */
class Change: public Job {
    Q_OBJECT
    Q_PROPERTY(QObject * receiver WRITE setReceiver)
    Q_PROPERTY(QString slot     WRITE setSlot)
    Q_PROPERTY(QString activity READ activity WRITE setActivity)

public:
    enum Type {
        Information,
        Warning,
        Error
    };

    DECLARE_JOB_FACTORY(Change, (QObject * receiver, const QString & slot, const QString & activity));

    QString activity() const;
    void setActivity(const QString & activity);

    void setReceiver(QObject * receiver);
    void setSlot(const QString & slot);

    virtual void start() _override;

private:
    QObject * m_receiver;
    QString m_slot;
    QString m_activity;

};

inline Change::Factory * change(QObject * receiver, const QString & slot, const QString & activity)
{
    return new Change::Factory(receiver, slot, activity);
}

} // namespace Activity
} // namespace Jobs

#endif // JOBS_ACTIVITY_START_H

