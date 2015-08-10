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
import "./components" as Local

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

    property string activityId: ""

    property alias activityName: activityName.text
    property alias activityIconSource: iconButton.iconName
    property alias activityPrivate: checkPrivate.checked
    property alias activityShortcut: panelShortcut.keySequence

    signal accepted()
    signal canceled()

    //////////////////////////////////////////////////////////////////////////

    visible: false

    height: content.height

    Local.IconChooser {
        id: iconButton

        anchors {
            left:   parent.left
            top:    parent.top

            topMargin: units.smallSpacing
        }
    }

    Column {
        id: content

        spacing: units.smallSpacing

        width: Math.max(checkPrivate.width, panelShortcut.width)

        anchors {
            top: parent.top
            right: parent.right
            left: iconButton.right
            leftMargin: units.largeSpacing
        }

        Local.NameChooser {
            id: activityName

            width: panelShortcut.width

            anchors {
                left:  parent.left
            }
        }

        QtControls.CheckBox {
            id: checkPrivate

            text: "Private - do not track usage for this activity"
        }

        Local.ShortcutChooser {
            id: panelShortcut
        }

        Local.DialogButtons {
            acceptText: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
            acceptIcon: "dialog-ok-apply"

            onAccepted: {
                root.accepted();
                root.close();
            }

            onCanceled: {
                root.canceled();
                root.close();
            }
        }
    }
}
