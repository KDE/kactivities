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

#ifndef ENCRYPTION_MOUNT_H_
#define ENCRYPTION_MOUNT_H_

#include "../Job.h"
#include "../JobFactory.h"

#include <QProcess>

namespace Jobs {
namespace Encryption {

/**
 * Mount
 */
class Mount: public Job {
    Q_OBJECT
    Q_PROPERTY(QString activity READ activity WRITE setActivity)
    Q_PROPERTY(int     action   READ action   WRITE setAction)

public:
    enum Action {
        InitializeAction,
        DeinitializeAction,
        MountAction,
        UnmountAction,
        UnmountExceptAction
    };

    DECLARE_JOB_FACTORY(Mount, (const QString & activity, int action));

    QString activity() const;
    void setActivity(const QString & activity);

    int action() const;
    void setAction(int value);

    virtual void start();

private Q_SLOTS:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString    m_activity;
    int        m_action;
    QProcess * m_process;

};

inline Mount::Factory * mount(const QString & activity, Mount::Action action)
{
    return new Mount::Factory(activity, action);
}

inline Mount::Factory * mount(const QString & activity, bool initialize = false)
{
    return new Mount::Factory(activity,
            initialize ? Mount::InitializeAction : Mount::MountAction
            );
}

inline Mount::Factory * unmount(const QString & activity, bool deinitialize = false)
{
    return new Mount::Factory(activity,
            deinitialize ? Mount::DeinitializeAction : Mount::UnmountAction);
}

} // namespace Jobs
} // namespace Encryption

#endif // ENCRYPTION_MOUNT_H_

