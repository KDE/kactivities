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

#ifndef JOBS_UI_SET_BUSY_H
#define JOBS_UI_SET_BUSY_H

#include "../Job.h"
#include "../JobFactory.h"

namespace Jobs {
namespace Ui {

/**
 * SetBusy
 */
class SetBusy: public Job {
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy WRITE setBusy)

public:
    DECLARE_JOB_FACTORY(SetBusy, (bool busy = true));

    bool busy() const;
    void setBusy(bool value);

    virtual void start() _override;

private:
    bool    m_busy;

};

inline SetBusy::Factory * setBusy(bool busy = true) {
    return new SetBusy::Factory(busy);
}

} // namespace Ui
} // namespace Jobs

#endif // JOBS_UI_SET_BUSY_H

