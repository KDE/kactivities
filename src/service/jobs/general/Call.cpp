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

#include "Call.h"

#include <QDebug>

namespace Jobs {
namespace General {

Call::JOB_FACTORY(QObject * receiver, const QString & slot, const QString & argument, bool waitFinished)
{
    JOB_FACTORY_PROPERTY(receiver);
    JOB_FACTORY_PROPERTY(slot);
    JOB_FACTORY_PROPERTY(argument);
    JOB_FACTORY_PROPERTY(waitFinished);
}

QString Call::argument() const
{
    return m_argument;
}

void Call::setArgument(const QString & argument)
{
    m_argument = argument;
}

void Call::setReceiver(QObject * receiver)
{
    m_receiver = receiver;
}

void Call::setSlot(const QString & slot)
{
    m_slot = slot;
}

QObject * Call::receiver() const
{
    return m_receiver;
}

QString Call::slot() const
{
    return m_slot;
}

bool Call::waitFinished() const
{
    return m_waitFinished;
}

void Call::setWaitFinished(bool value)
{
    m_waitFinished = value;
}

void Call::start()
{
    if (m_receiver) {
        qDebug() << ">>> Calling the method" << m_slot << "with" << m_argument;

        QMetaObject::invokeMethod(m_receiver, m_slot.toAscii(),
                (m_waitFinished ? Qt::QueuedConnection : Qt::DirectConnection),
                Q_ARG(QString, m_argument));

    } else {
        qDebug() << ">>> Receiver is nullptr, failing";
        setError(1);
        setErrorText("There is no receiver registered to call");

    }

    emit emitResult();
}

} // namespace General
} // namespace Jobs

