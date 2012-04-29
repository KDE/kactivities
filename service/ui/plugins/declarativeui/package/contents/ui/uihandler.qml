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

Rectangle {
    id: main

    color: Qt.rgba(0, 0, 0, 0.35)
    visible: false

    width: 400
    height: 360

    property int mainIconSize: 64 + 32
    property int layoutPadding: 8

    Rectangle {
        id: opaqueBackground

        color: Qt.rgba(0, 0, 0, 1)
        visible: false
    }

    Timer {
        running: true
        repeat: false
        interval: 1000

        onTriggered: { main.visible = true }
    }

    MouseArea {
        onClicked: { uihandler.cancel() }

        anchors.fill: parent
    }

    PlasmaComponents.BusyIndicator {
        anchors.centerIn: parent
        running: visible
        visible: (uihandler.windowVisible && !dialogNewPassword.active && !dialogPassword.active && !dialogMessage.active && !dialogChoice.active)
    }

    NewPasswordDialog {
        id: dialogNewPassword

        anchors {
            fill: parent
            leftMargin: 50
            topMargin: 32
            rightMargin: 50
        }

        property bool active: false
        transform: Translate {
            y: dialogNewPassword.active ? 0 : main.height - dialogNewPassword.y
            Behavior on y { NumberAnimation { duration: 300 } }
        }

        title:                  i18n("Enter the password")
        passwordText1:          i18n("Password:")
        passwordText2:          i18n("Verify:")
        strengthText:           i18n("Password strength meter:")
        passwordsMatchText:     i18n("Passwords match")
        passwordsDontMatchText: i18n("Passwords do not match")
        okText:                 i18n("Save")
        cancelText:             i18n("Cancel")

        onPasswordChosen: uihandler.returnPassword(password)

        onCanceled: uihandler.cancel()

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    PasswordDialog {
        id: dialogPassword

        anchors.centerIn: parent

        property bool active: false
        transform: Translate {
            y: dialogPassword.active ? 0 : main.height - dialogNewPassword.y
            Behavior on y { NumberAnimation { duration: 300 } }
        }

        title:      i18n("Enter the password")
        okText:     i18n("Unlock")
        cancelText: i18n("Cancel")

        onPasswordChosen: uihandler.returnPassword(password)
        onCanceled: uihandler.cancel()

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    ChoiceDialog {
        id: dialogChoice

        anchors.centerIn: parent

        property bool active: false

        transform: Translate {
            y: dialogChoice.active ? 0 : main.height - dialogChoice.y
            Behavior on y { NumberAnimation { duration: 300 } }
        }

        title:      i18n("Choice")

        onChoiceChosen: uihandler.returnChoice(-1 - index)
        onCanceled: uihandler.cancel()

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    MessageDialog {
        id: dialogMessage

        property bool active: false
        transform: Translate {
            y: dialogMessage.active ? 0 : main.height - dialogNewPassword.y
            Behavior on y { NumberAnimation { duration: 300 } }
        }

        // Just so that clicking inside this are doesn't call cancel
        MouseArea { anchors.fill: parent; z: -1; onClicked: {} }
    }

    Connections {
        target: uihandler

        // void message(const QString & message);
        onMessage: {
            dialogPassword.text = message
            dialogMessage.active = true
            dialogChoice.active = false
            dialogNewPassword.active = false
            dialogPassword.active = false
        }

        // void ask(const QString & title, const QString & message, const QStringList & choices);
        onAsk: {
            print("Asking....")
            dialogChoice.title = title
            dialogChoice.message = message
            dialogChoice.choices = choices
            dialogChoice.active = true

            dialogMessage.active = false
            dialogNewPassword.active = false
            dialogPassword.active = false
        }

        // void askPassword(const QString & title, const QString & message, bool newPassword, bool unlockMode);
        onAskPassword: {
            print("Asking for password....")
            if (newPassword) {
                dialogNewPassword.password = ""
                dialogNewPassword.passwordConfirmation = ""
                dialogNewPassword.active = true
                dialogMessage.active = false
                dialogChoice.active = false
                dialogPassword.active = false

            } else {
                opaqueBackground.visible = unlockMode
                dialogPassword.title = title
                dialogPassword.message = message
                dialogPassword.password = ""
                dialogPassword.active = true
                dialogMessage.active = false
                dialogChoice.active = false
                dialogNewPassword.active = false

            }
        }

        onHideAll: {
            dialogMessage.active = false
            dialogChoice.active = false
            dialogNewPassword.active = false
            dialogPassword.active = false
            opaqueBackground.visible = false
        }
    }
}
