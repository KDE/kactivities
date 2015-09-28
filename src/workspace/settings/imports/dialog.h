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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "utils/d_ptr.h"

class Dialog: public QDialog {
    Q_OBJECT

public:
    Dialog(QObject *parent = Q_NULLPTR);
    Dialog(const QString &activityId, QObject *parent = Q_NULLPTR);

    ~Dialog();

    void initUi();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    D_PTR;

};

#endif // DIALOG_H

