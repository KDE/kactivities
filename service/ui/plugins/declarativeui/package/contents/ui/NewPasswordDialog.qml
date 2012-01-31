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

    property alias password:   textPassword1.text
    property alias title: labelTitle.text
    property alias passwordText1: labelPassword1.text
    property alias passwordText2: labelPassword2.text
    property alias strengthText:  labelPasswordStrength.text
    property string passwordsMatchText
    property string passwordsDontMatchText
    property alias okText:     buttonOk.text
    property alias cancelText: buttonCancel.text

    signal canceled
    signal passwordChosen (string password)

    width: 300
    height: 350

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
                    verticalCenter: parent.verticalCenter
                    left: iconTitle.right
                    leftMargin: main.layoutPadding
                }
            }
        }

        // Password fields

        Item {
            id: panelPasswords
            height: main.mainIconSize

            anchors {
                top: panelTop.bottom
                left: parent.left
                right: parent.right
                topMargin: main.layoutPadding
            }

            PlasmaComponents.TextField {
                id: textPassword1

                anchors {
                    top:  parent.top
                    right: parent.right
                }

                echoMode: TextInput.Password
            }

            PlasmaComponents.TextField {
                id: textPassword2

                anchors {
                    top:  labelPassword2.top
                    right: parent.right
                }

                echoMode: TextInput.Password
            }

            PlasmaComponents.Label {
                id: labelPassword1

                anchors {
                    top:   parent.top
                    right: textPassword1.left
                    rightMargin: main.layoutPadding
                }
            }

            PlasmaComponents.Label {
                id: labelPassword2

                anchors {
                    top: labelPassword1.bottom
                    right: textPassword2.left
                    rightMargin: main.layoutPadding
                }
            }
        }

        // Status

        Item {
            id: panelStrength
            height: 32

            anchors {
                top: panelPasswords.bottom
                left: parent.left
                right: parent.right
                topMargin: main.layoutPadding
            }

            PlasmaComponents.Label {
                id: labelPasswordStrength

                anchors {
                    left: parent.left
                    top:  parent.top
                }
            }

            PlasmaComponents.ProgressBar {
                id: progressPasswordStrength
                maximumValue: 100
                minimumValue: 0
                value: (textPassword1.text.length == 0) ? 0 : passwordStrength(textPassword1.text)

                function passwordStrength(text) {
                    var value = 50;

                    if (text.length < 8) {
                        value = (50 * text.length / 8)
                    }

                    var upper = 0;
                    var nonletter = 0;
                    var numeric = 0;

                    for (var i = 0; i < text.length; i++) {
                        var c = text[i];
                        if (c >= 'A' && c <= 'Z') {
                            upper = 1;

                        } else if (c >= '0' && c <= '9') {
                            numeric = 1;

                        } else if (c < 'a' && c > 'z') {
                            nonletter = 1;

                        }
                    }

                    return value + 17 * upper + 17 * numeric + 16 * nonletter;
                }

                anchors {
                    left: labelPasswordStrength.right
                    right: parent.right
                    verticalCenter: labelPasswordStrength.verticalCenter
                    leftMargin: main.layoutPadding
                }
            }
        }

        Item {
            id: panelMatching
            height: 32

            anchors {
                top: panelStrength.bottom
                left: parent.left
                right: parent.right
                topMargin: main.layoutPadding
            }

            PlasmaComponents.Label {
                id: labelMatching
                text: (textPassword1.text == textPassword2.text) ? main.passwordsMatchText : main.passwordsDontMatchText

                anchors {
                    left: parent.left
                    top:  parent.top
                }
            }

            QIconItem {
                id: iconMatching
                height: 24
                width:  24
                icon: (textPassword1.text == textPassword2.text) ? "dialog-ok" : "dialog-cancel"

                anchors {
                    right: parent.right
                    verticalCenter: labelMatching.verticalCenter
                    leftMargin: main.layoutPadding
                }
            }
        }

        // Buttons

        Item {
            id: buttons
            height: 32

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            PlasmaComponents.Button {
                id: buttonOk
                enabled: (textPassword1.text.length != 0) && (textPassword1.text == textPassword2.text)

                anchors {
                    bottomMargin: 4
                    rightMargin: 4
                    leftMargin: 4

                    left: parent.left
                    right: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                }

                onClicked: main.passwordChosen(textPassword1.text)
            }

            PlasmaComponents.Button {
                id: buttonCancel

                anchors {
                    bottomMargin: 4
                    rightMargin: 4
                    leftMargin: 4

                    right: parent.right
                    left: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                }

                onClicked: main.canceled
            }
        }
    }
}
