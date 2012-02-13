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
    property alias passwordConfirmation:   textPassword2.text
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

    width: 350
    height: 400

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        anchors.fill: parent

        imagePath: "dialogs/background"
	enabledBorders: "TopBorder|LeftBorder|RightBorder"
    }

    anchors.centerIn: parent

    Item {
        anchors {
            fill: parent
            leftMargin: backgroundFrame.margins.left
            topMargin: backgroundFrame.margins.top
            rightMargin: backgroundFrame.margins.right
            bottomMargin: backgroundFrame.margins.bottom
        }

        PlasmaCore.FrameSvgItem {
            id: titleFrame
            imagePath: "widgets/extender-dragger"
            prefix: "root"
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                leftMargin: parent.margins.left
                rightMargin: parent.margins.right
                topMargin: parent.margins.top
            }
            height: titleLabel.height + margins.top + margins.bottom
            PlasmaComponents.Label {
                id: titleLabel
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: i18n("New activity password")
                font.pointSize: theme.defaultFont.pointSize * 1.1
                font.weight: Font.Bold
                style: Text.Raised
                styleColor: Qt.rgba(1,1,1,0.8)
                height: paintedHeight
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    topMargin: parent.margins.top
                    leftMargin: height + 2
                    rightMargin: height + 2
                }
            }
        }

        
        
        // Top row - icon and the text

        Item {
            id: panelTop
            height: main.mainIconSize

            anchors {
                top: titleFrame.bottom
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

            Column {
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: iconTitle.right
                    right: parent.right
                    leftMargin: main.layoutPadding
                }
                spacing: 8
                PlasmaComponents.Label {
                    text: i18n("Protecting an activity will make your private data safe. Private activities require the user to enter a password to be accessed. The content added to the activity will not be shared with other activities and won't appear in search results. Private data will be encrypted.")
                    wrapMode: Text.WordWrap
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }
                PlasmaComponents.Label {
                    id: labelTitle
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        // Password fields

        Grid {
            id: panelPasswords
            height: childrenRect.height
            rows: 3

            anchors {
                top: panelTop.bottom
                horizontalCenter: parent.horizontalCenter
                topMargin: main.layoutPadding
            }
            width: childrenRect.width

            PlasmaComponents.Label {
                id: labelPassword1

                anchors {
                    right: textPassword1.left
                    rightMargin: main.layoutPadding
                }
            }

            PlasmaComponents.TextField {
                id: textPassword1
                echoMode: !showPasswordCheckBox.checked ? TextInput.Password : TextInput.Normal
            }

            PlasmaComponents.Label {
                id: labelPassword2

                anchors {
                    right: textPassword2.left
                    rightMargin: main.layoutPadding
                }
            }

            PlasmaComponents.TextField {
                id: textPassword2
                echoMode: !showPasswordCheckBox.checked ? TextInput.Password : TextInput.Normal
            }
            
            PlasmaComponents.Label {
                text: i18n("Show password:")

                anchors {
                    right: textPassword2.left
                    rightMargin: main.layoutPadding
                }
            }

            PlasmaComponents.CheckBox {
                id: showPasswordCheckBox
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
                leftMargin: 100
                rightMargin:100
            }

            Item {
                id: panelMatching
                height: 32

                
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: main.layoutPadding
                }

                PlasmaComponents.Label {
                    id: labelMatching
                    text: (textPassword1.text == textPassword2.text) ? main.passwordsMatchText : main.passwordsDontMatchText

                    anchors {
                        right: iconMatching.left
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

            PlasmaComponents.Label {
                id: labelPasswordStrength
                anchors {
                    left: parent.left
                    top: panelMatching.bottom
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
                    top: labelPasswordStrength.bottom
                    left: parent.left
                    right: parent.right
                    topMargin: main.layoutPadding
                }
            }
        }

        // Buttons

        Row {
            id: buttons
            height: 48
            spacing: 4

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
            }

            PlasmaComponents.Button {
                id: buttonOk
                enabled: (textPassword1.text.length != 0) && (textPassword1.text == textPassword2.text)

                onClicked: main.passwordChosen(textPassword1.text)
            }

            PlasmaComponents.Button {
                id: buttonCancel

                onClicked: main.canceled()
            }
        }
    }
}
