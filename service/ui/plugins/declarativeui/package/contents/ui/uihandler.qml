/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2012 Ivan Čukić <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.mobilecomponents 0.1 as MobileComponents
import org.kde.qtextracomponents 0.1

Image {
    id: main

    source: "image://appbackgrounds/standard"
    fillMode: Image.Tile

    width: 400
    height: 360

    property int mainIconSize: 64 + 32
    property int layoutPadding: 8

    NewPasswordDialog {
        anchors.centerIn: parent

        id: dialogNewPassword
        visible: false

        title:                  "Enter the password"
        passwordText1:          "Password:"
        passwordText2:          "Verify:"
        strengthText:           "Password strength meter:"
        passwordsMatchText:     "Passwords match"
        passwordsDontMatchText: "Passwords don't match"
        okText:                 "Ok"
        cancelText:             "Cancel"
    }

    PasswordDialog {
        anchors.centerIn: parent

        id: dialogPassword
        visible: true

        title:      "Enter the password"
        okText:     "Unlock"
        cancelText: "Dismiss"
    }

    MessageDialog {
        id: dialogMessage
        text: "Failed to unlock the activity.\nProbably due to a wrong password."
        opacity: 1

        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }
        }
    }

    MouseArea {
        onClicked: dialogMessage.opacity = 0

        anchors.fill: parent
    }
}
