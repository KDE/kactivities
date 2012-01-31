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

Item {
    id: main
    property int mainIconSize: 64 + 32
    property int layoutPadding: 8

    property alias title:      labelTitle.text
    property alias password:   textPassword.text
    property alias okText:     buttonOk.text
    property alias cancelText: buttonCancel.text

    signal canceled
    signal passwordChosen (string password)

    width: 300
    height: 180

    Rectangle {
        anchors.fill: parent

        color: "white"
        border.color: "gray"
        border.width: 1
        radius: 4
    }

    anchors.centerIn: parent

    Item {
        anchors {
            fill: parent
            leftMargin: main.layoutPadding
            topMargin: main.layoutPadding
            rightMargin: main.layoutPadding
            bottomMargin: main.layoutPadding
        }

        // Top row - icon and the text

        Item {
            id: panelTop
            height: main.mainIconSize

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right

                leftMargin: main.layoutPadding
                topMargin: main.layoutPadding
                rightMargin: main.layoutPadding
            }

            QIconItem {
                id: iconTitle
                icon: "dialog-password"

                width: main.mainIconSize
                height: main.mainIconSize

                anchors {
                    top: parent.top
                    left: parent.left
                    bottom: parent.bottom
                }
            }

            PlasmaComponents.Label {
                id: labelTitle

                anchors {
                    bottom: parent.verticalCenter
                    left: iconTitle.right
                    leftMargin: main.layoutPadding
                }
            }

            PlasmaComponents.TextField {
                id: textPassword

                anchors {
                    top:   parent.verticalCenter
                    right: parent.right
                    left:  iconTitle.right
                    leftMargin: main.layoutPadding
                }

                echoMode: TextInput.Password
            }
        }

        // Buttons

        Item {
            id: buttons
            height: 48

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            PlasmaComponents.Button {
                id: buttonOk
                enabled: (textPassword.text.length != 0)

                anchors {
                    bottomMargin: 4
                    rightMargin: 4
                    leftMargin: 4
                    topMargin: 4

                    left: parent.left
                    right: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                }

                onClicked: main.passwordChosen(textPassword.text)
            }

            PlasmaComponents.Button {
                id: buttonCancel

                anchors {
                    bottomMargin: 4
                    rightMargin: 4
                    leftMargin: 4
                    topMargin: 4

                    right: parent.right
                    left: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                }

                onClicked: main.canceled()
            }
        }
    }
}
