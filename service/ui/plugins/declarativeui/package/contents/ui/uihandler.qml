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

    MouseArea {
        onClicked: { dialogMessage.opacity = 0; uihandler.cancel() }

        anchors.fill: parent
    }

    PlasmaComponents.BusyIndicator {
        anchors.centerIn: parent
        running: visible
        visible: (dialogNewPassword.opacity == 0 && dialogPassword.opacity == 0 && dialogMessage.opacity == 0)
    }

    NewPasswordDialog {
        id: dialogNewPassword

        anchors.centerIn: parent

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        title:                  "Enter the password"
        passwordText1:          "Password:"
        passwordText2:          "Verify:"
        strengthText:           "Password strength meter:"
        passwordsMatchText:     "Passwords match"
        passwordsDontMatchText: "Passwords don't match"
        okText:                 "Ok"
        cancelText:             "Cancel"

        onPasswordChosen: uihandler.returnPassword(password)

        onCanceled: uihandler.cancel()

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    PasswordDialog {
        id: dialogPassword

        anchors.centerIn: parent

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        title:      "Enter the password"
        okText:     "Unlock"
        cancelText: "Dismiss"

        onPasswordChosen: uihandler.returnPassword(password)
        onCanceled: uihandler.cancel()

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    MessageDialog {
        id: dialogMessage

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    Connections {
        target: uihandler

        // void message(const QString & message);
        onMessage: {
            dialogMessage.text = message
            dialogMessage.opacity = 1
        }

        // void askPassword(const QString & title, const QString & message, bool newPassword);
        onAskPassword: {
            if (newPassword) {
                dialogNewPassword.password = ""
                dialogNewPassword.passwordConfirmation = ""
                dialogNewPassword.opacity = 1

            } else {
                dialogPassword.password = ""
                dialogPassword.opacity = 1

            }
        }

        onHideAll: {
            dialogMessage.opacity = 0
            dialogNewPassword.opacity = 0
            dialogPassword.opacity = 0
        }
    }
}
