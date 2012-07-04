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

#ifndef JOBS_GENERAL_CALL_H
#define JOBS_GENERAL_CALL_H

#include "../Job.h"
#include "../JobFactory.h"

namespace Jobs {
namespace General {

/**
 * Call
 */
class Call: public Job {
    Q_OBJECT
    Q_PROPERTY(QObject * receiver     READ receiver     WRITE setReceiver)
    Q_PROPERTY(QString   slot         READ slot         WRITE setSlot)
    Q_PROPERTY(QString   argument     READ argument     WRITE setArgument)
    Q_PROPERTY(bool      waitFinished READ waitFinished WRITE setWaitFinished)

public:
    enum Type {
        Information,
        Warning,
        Error
    };

    DECLARE_JOB_FACTORY(Call, (QObject * receiver, const QString & slot, const QString & argument, bool waitFinished));

    QString argument() const;
    void setArgument(const QString & argument);

    void setReceiver(QObject * receiver);
    QObject * receiver() const;

    void setSlot(const QString & slot);
    QString slot() const;

    void setWaitFinished(bool value);
    bool waitFinished() const;

    virtual void start() _override;

private:
    QObject * m_receiver;
    QString m_slot;
    QString m_argument;
    bool    m_waitFinished;

};

inline Call::Factory * call(QObject * receiver, const QString & slot,
        const QString & argument, bool waitFinished = false)
{
    return new Call::Factory(receiver, slot, argument, waitFinished);
}

} // namespace General
} // namespace Jobs

#endif // JOBS_GENERAL_CALL_H

