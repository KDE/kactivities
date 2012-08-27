/*   vim:set foldenable foldmethod=marker:
 *
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

import QtQuick 1.1
import org.kde.qtextracomponents 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Flow {
    id: main

    /* property declarations --------------------------{{{ */
    property int minimumHeight: 100
    /* }}} */

    /* signal declarations ----------------------------{{{ */
    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    /* }}} */

    /* object properties ------------------------------{{{ */
    spacing: 16
    anchors {
        fill         : parent
        rightMargin  : 8
        bottomMargin : 8
        leftMargin   : 8
    }

    height: Math.max(childrenRect.height, minimumHeight)

    opacity: if (applicationModel.enabled) {1} else {.3}
    Behavior on opacity { NumberAnimation { duration: 150 } }
    /* }}} */

    /* child objects ----------------------------------{{{ */
    Repeater {
        model: applicationModel
        Column {
            id: item

            property bool blocked: model.blocked

            Item {
                id: mainIcon

                width  : 64 + 20
                height : 64 + 20

                QIconItem {
                    id: icon
                    icon: model.icon

                    anchors.fill    : parent
                    anchors.margins : 10

                    opacity: if (item.blocked) {0.5} else {1.0}
                    Behavior on opacity { NumberAnimation { duration: 150 } }
                }

                QIconItem {
                    id: iconNo
                    icon: "dialog-cancel"

                    anchors {
                        right  : parent.right
                        bottom : parent.bottom
                    }

                    width   : 48
                    height  : 48
                    opacity : (1 - icon.opacity) * 2
                }

                MouseArea {
                    onClicked: applicationModel.toggleApplicationBlocked(model.index)
                    anchors.fill: parent
                }
            }

            Text {
                elide   : Text.ElideRight
                width   : mainIcon.width

                text    : model.title;
                opacity : icon.opacity

                anchors.margins          : 10
                anchors.horizontalCenter : parent.horizontalCenter
                horizontalAlignment      : Text.AlignHCenter
            }
        }
    }
    /* }}} */

    /* states -----------------------------------------{{{ */
    /* }}} */

    /* transitions ------------------------------------{{{ */
    /* }}} */
}

