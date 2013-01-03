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

#include "Change.h"

#include <QDebug>

namespace Jobs {
namespace Activity {

Change::JOB_FACTORY(QObject * receiver, const QString & slot, const QString & activity)
{
    JOB_FACTORY_PROPERTY(receiver);
    JOB_FACTORY_PROPERTY(slot);
    JOB_FACTORY_PROPERTY(activity);
}

QString Change::activity() const
{
    return m_activity;
}

void Change::setActivity(const QString & activity)
{
    m_activity = activity;
}

void Change::setReceiver(QObject * receiver)
{
    m_receiver = receiver;
}

void Change::setSlot(const QString & slot)
{
    m_slot = slot;
}

void Change::start()
{
    if (m_receiver) {
        qDebug() << ">>> Calling the method to set activity" << m_activity << "slot" << m_slot;
        bool ret = QMetaObject::invokeMethod(m_receiver, m_slot.toAscii(), Qt::QueuedConnection, Q_ARG(QString, m_activity));
        qDebug() << ret;

    } else {
        qDebug() << ">>> Receiver is nullptr, failing";
        setError(1);
        setErrorText("There is no receiver registered to signal the change activity");

    }

    emit emitResult();
}

} // namespace Activity
} // namespace Jobs

