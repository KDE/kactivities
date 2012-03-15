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

PlasmaCore.FrameSvgItem {
    id: main
    property int mainIconSize: 64 + 32
    property int layoutPadding: 16

    property alias title:      labelTitle.text
    property alias message:    labelMessage.text
    property alias choices:    listChoices.model

    signal canceled
    signal choiceChosen (int index)


    imagePath: "dialogs/background"
    enabledBorders: "TopBorder|LeftBorder|RightBorder"


    anchors {
        fill: parent
        leftMargin: 50
        rightMargin: 50
        topMargin: 50
    }

    // Top row - icon and the text

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
        height: labelTitle.height + margins.top + margins.bottom
        PlasmaComponents.Label {
            id: labelTitle
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
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

    Item {
        id: contents
        anchors {
            centerIn: parent
        }

        width: 400 // panelTop.width + main.layoutPadding
        height: 400 // titleFrame.height + panelTop.height + main.layoutPadding


        Column {
            id: panelTop
            anchors {
                top: titleFrame.bottom
                topMargin: 2
                horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 2
                QIconItem {
                    id: iconTitle
                    icon: "dialog-warning"

                    width: main.mainIconSize
                    height: main.mainIconSize
                }
                PlasmaComponents.Label {
                    id: labelMessage
                    anchors.verticalCenter: parent.verticalCenter
                    wrapMode: Text.Wrap
                }
            }

            ListView {
                id: listChoices

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: panelTop.bottom
                }

                width: 400
                height: 300

                delegate: PlasmaComponents.ListItem {
                    PlasmaComponents.Label {
                        anchors.fill: parent

                        text: modelData
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked:    main.choiceChosen(index)
                    }
                }
            }
        }
    }
}
