/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#pragma once

#include <QMainWindow>

#include <activitiesmodel.h>
#include <consumer.h>

namespace Ui {
    class MainWindow;
}

class Window: public QMainWindow {
    Q_OBJECT

public:
    Window();
    ~Window();

protected:
    void showEvent(QShowEvent * event) override;

private:
    Ui::MainWindow *ui;
    KActivities::Consumer *activities;
    KActivities::ActivitiesModel *modelRunningActivities;
    KActivities::ActivitiesModel *modelStoppedActivities;
};

