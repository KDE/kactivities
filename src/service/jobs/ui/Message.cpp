/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Ui Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Ui Public License for more details
 *
 *   You should have received a copy of the GNU Ui Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Message.h"
#include "../../ui/Ui.h"

#include <KDebug>

namespace Jobs {
namespace Ui {

Message::JOB_FACTORY(const QString & title, const QString & message, int type)
{
    JOB_FACTORY_PROPERTY(title);
    JOB_FACTORY_PROPERTY(message);
    Q_UNUSED(type);
    // JOB_FACTORY_PROPERTY(type);
}

QString Message::message() const
{
    return m_message;
}

void Message::setMessage(const QString & message)
{
    m_message = message;
}

QString Message::title() const
{
    return m_title;
}

void Message::setTitle(const QString & title)
{
    m_title = title;
}


void Message::start()
{
    kDebug() << ">>> Writing the message out" << m_message;
    Ui::message(m_title, m_message);
    emit emitResult();
}

} // namespace Ui
} // namespace Jobs

