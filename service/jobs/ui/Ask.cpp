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

#include "Ask.h"
#include "../../ui/Ui.h"

#include <KDebug>

namespace Jobs {
namespace Ui {

Ask::JOB_FACTORY(const QString & title, const QString & message, const QStringList & choices)
{
    JOB_FACTORY_PROPERTY(title);
    JOB_FACTORY_PROPERTY(message);
    JOB_FACTORY_PROPERTY(choices);
}

QString Ask::message() const
{
    return m_message;
}

void Ask::setMessage(const QString & message)
{
    m_message = message;
}

QString Ask::title() const
{
    return m_title;
}

void Ask::setTitle(const QString & title)
{
    m_title = title;
}

QStringList Ask::choices() const
{
    return m_choices;
}

void Ask::setChoices(const QStringList & choices)
{
    m_choices = choices;
}

void Ask::start()
{
    kDebug() << ">>> Ask" << m_message << m_choices;

    // Needed due to namespace collision with Jobs::Ui
    ::Ui::ask(m_title, m_message, m_choices,
            this, "choiceChosen");
}

void Ask::choiceChosen(int choice)
{
    setError(choice);
    emitResult();
}

} // namespace Ui
} // namespace Jobs

