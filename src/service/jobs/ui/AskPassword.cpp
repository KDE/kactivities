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

#include "AskPassword.h"
#include "../../ui/Ui.h"

#include <KDebug>

namespace Jobs {
namespace Ui {

AskPassword::JOB_FACTORY(const QString & title, const QString & message, bool shouldVerify, bool unlockMode)
{
    JOB_FACTORY_PROPERTY(title);
    JOB_FACTORY_PROPERTY(message);
    JOB_FACTORY_PROPERTY(shouldVerify);
    JOB_FACTORY_PROPERTY(unlockMode);
}

QString AskPassword::message() const
{
    return m_message;
}

void AskPassword::setMessage(const QString & message)
{
    m_message = message;
}

QString AskPassword::title() const
{
    return m_title;
}

void AskPassword::setTitle(const QString & title)
{
    m_title = title;
}

bool AskPassword::shouldVerify() const
{
    return m_shouldVerify;
}

void AskPassword::setShouldVerify(bool value)
{
    m_shouldVerify = value;
}

bool AskPassword::unlockMode() const
{
    return m_unlockMode;
}

void AskPassword::setUnlockMode(bool value)
{
    m_unlockMode = value;
}

void AskPassword::start()
{
    kDebug() << ">>> AskPasswording for the password" << m_message;

    // Needed due to namespace collision with Jobs::Ui
    ::Ui::askPassword(m_title, m_message, m_shouldVerify, m_unlockMode,
            this, "passwordReturned");
}

void AskPassword::passwordReturned(const QString & password)
{
    kDebug() << "returned password";
    global()->setProperty(passwordField(), password);

    if (password.isEmpty()) {
        setError(1);
        setErrorText("Canceled, or a blank password entered");
    }

    emit emitResult();
}

const char * AskPassword::passwordField()
{
    return "Ui__AskPassword__password";
}

} // namespace Ui
} // namespace Jobs

