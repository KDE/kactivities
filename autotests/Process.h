/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef PROCESS_H
#define PROCESS_H

#include <common/test.h>

#include <controller.h>

class QProcess;
class QTemporaryDir;

namespace Process {
    enum Action {
        Start,
        Stop,
        Kill,
        Crash
    };

    class Modifier : public Test {
        Q_OBJECT
    public:
        Modifier(Action action);

    private Q_SLOTS:
        void initTestCase();
        void testProcess();
        void cleanupTestCase();

    private:
        Action m_action;
        static QProcess *s_process;
        static QTemporaryDir *s_tempDir;

    };

    Modifier *exec(Action action);

} // namespace Process


#endif /* PROCESS_H */

