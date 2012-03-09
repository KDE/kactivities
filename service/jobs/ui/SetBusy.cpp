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

#include "SetBusy.h"
#include "../../ui/Ui.h"

#include <KDebug>

namespace Jobs {
namespace Ui {

SetBusy::JOB_FACTORY(bool busy)
{
    JOB_FACTORY_PROPERTY(busy);
}

bool SetBusy::busy() const
{
    return m_busy;
}

void SetBusy::setBusy(bool value)
{
    m_busy = value;
}


void SetBusy::start()
{
    // Needed due to namespace collision with Jobs::Ui
    ::Ui::setBusy(m_busy);
    emit emitResult();
}

} // namespace Ui
} // namespace Jobs

