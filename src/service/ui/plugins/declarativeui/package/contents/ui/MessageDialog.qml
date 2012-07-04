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
    property int mainIconSize: 48
    property int layoutPadding: 8

    property alias text: labelTitle.text

    width: 400
    height: mainLayout.height + backgroundFrame.margins.top + backgroundFrame.margins.bottom

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        anchors.fill: parent

        imagePath: "dialogs/background"
    }

    anchors.centerIn: parent

    Row {
        id: mainLayout
        spacing: main.layoutPadding
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
            leftMargin: backgroundFrame.margins.left
            rightMargin: backgroundFrame.margins.right
            topMargin: backgroundFrame.margins.top
        }
        QIconItem {
            id: iconTitle
            icon: "dialog-error"

            width: main.mainIconSize
            height: main.mainIconSize

            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }

        PlasmaComponents.Label {
            id: labelTitle

            wrapMode: Text.WordWrap
            width: mainLayout.width - iconTitle.width - main.layoutPadding
            anchors {
                verticalCenter: parent.verticalCenter
            }
        }
    }
}
