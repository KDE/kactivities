/*   vim:set foldmethod=marker:
 *
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

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrols 2.0 as KQuickControls

import "static.js" as S

Item {
    id: root

    function open() {
        visible = true;
        S.dialogOpened(root);
    }

    function close() {
        visible = false;
        S.dialogClosed(root);
    }

    property alias activityName: activityNameText.text
    property alias activityIconSource: iconButton.iconName
    property alias activityPrivate: checkPrivate.checked
    property alias activityShortcut: buttonKeyShorcut.keySequence

    signal accepted()
    signal canceled()

    //////////////////////////////////////////////////////////////////////////

    visible: false

    height: content.height

    QtControls.Button {
        id: iconButton

        iconName: model.iconSource

        width:  height
        height: units.iconSizes.medium

        anchors {
            left:   parent.left
            top:    parent.top

            topMargin: units.smallSpacing
            bottomMargin: units.smallSpacing
        }
    }

    Column {
        id: content

        spacing: units.smallSpacing

        anchors {
            top: parent.top
            right: parent.right
            left: iconButton.right
            leftMargin: units.largeSpacing
        }

        Item {
            height: header.height + 2 * units.smallSpacing

            anchors {
                left:  parent.left
                right: parent.right
            }

            QtControls.TextField {
                id: activityNameText

                text: model.name

                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }

        QtControls.CheckBox {
            id: checkPrivate

            text: "Private - do not track usage for this activity"
        }

        Row {
            spacing: units.smallSpacing

            QtControls.Label {
                anchors.verticalCenter: parent.verticalCenter
                text: "Shortcut for switching to this activity: "
            }

            KQuickControls.KeySequenceItem {
                id: buttonKeyShorcut
                keySequence: plasmoid.globalShortcut
                onKeySequenceChanged: {
                    root.configurationChanged();
                }
            }
        }

        Row {
            spacing: units.smallSpacing

            QtControls.Button {
                id: buttonApply

                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                iconName: "dialog-ok-apply"

                onClicked: {
                    root.accepted();
                    root.close();
                }
            }

            QtControls.Button {
                id: buttonCancel

                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                iconName: "dialog-cancel"

                onClicked: {
                    root.canceled();
                    root.close();
                }
            }
        }
    }
}
