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

    signal accepted()
    signal canceled()

    //////////////////////////////////////////////////////////////////////////

    visible: false

    height: content.height + units.smallSpacing * 2

    Column {
        id: content

        spacing: units.smallSpacing

        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
            topMargin: units.smallSpacing
            leftMargin: units.iconSizes.medium + units.largeSpacing
        }

        QtControls.Label {
            text: "Are you sure you want to delete this activity?"
        }

        Row {
            spacing: units.smallSpacing

            QtControls.Button {
                id: buttonApply

                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Delete")
                iconName: "trash-empty"

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
