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

#include "Mount.h"
#include "Common.h"

#include <jobs/ui/AskPassword.h>

#include <KDebug>

namespace Jobs {
namespace Encryption {

Mount::JOB_FACTORY(const QString & activity, int action)
{
    JOB_FACTORY_PROPERTY(activity);
    JOB_FACTORY_PROPERTY(action);
}

QString Mount::activity() const
{
    return m_activity;
}

void Mount::setActivity(const QString & activity)
{
    m_activity = activity;
}

int Mount::action() const
{
    return m_action;
}

void Mount::setAction(int value)
{
    m_action = value;
}

void Mount::start()
{
    kDebug() << m_activity << m_action;

    m_process = nullptr;

    switch (m_action) {
        case Mount::MountAction:
            m_process = Common::execMount(
                    m_activity,
                    global()->property(Jobs::Ui::AskPassword::passwordField()).toString()
                );

            break;

        case Mount::UnmountAction:
            m_process = Common::execUnmount(
                    m_activity
                );

            break;

        case Mount::UnmountExceptAction:
            Common::unmountAllExcept(m_activity);

            break;

        default:
            setError(1);
            setErrorText("No valid action specified");
    }

    if (m_process) {
        kDebug() << "Connecting the process";
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(processFinished(int, QProcess::ExitStatus)));

    } else {
        kDebug() << "No process. Saying that we are done.";
        emit emitResult();

    }
}

void Mount::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    if (m_process->exitCode() != 0 || m_process->exitStatus() != QProcess::NormalExit) {
        setError(1);
        setErrorText("Mounting failed");

    }

    emit emitResult();
}

} // namespace Enc
} // namespace Jobs

