/*   vim:set foldenable foldmethod=marker:
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
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.activities 0.1 as Activities
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    anchors.fill: parent

    PlasmaComponents.Button {
        id: buttonCreateActivity

        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Create activity...")
        iconSource: "list-add"

        anchors {
            top: parent.top
            left: parent.left
        }
    }

    QtControls.ScrollView {
        anchors {
            top: buttonCreateActivity.bottom
            topMargin: units.smallSpacing
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ListView {
            width: parent.width
            // anchors.fill: parent

            model: Activities.ActivityModel {
                id: kactivities
            }

            SystemPalette {
                id: palette
                colorGroup: SystemPalette.Active
            }

            delegate: Rectangle {
                width: parent.width
                height: icon.height + units.smallSpacing * 2

                color: (model.index % 2 == 0) ? palette.base : palette.alternateBase

                QIconItem {
                    id: icon
                    icon: model.icon

                    width: units.iconSizes.medium
                    height: units.iconSizes.medium

                    anchors {
                        left:   parent.left
                        top:    parent.top
                        bottom: parent.bottom

                        topMargin: units.smallSpacing
                        bottomMargin: units.smallSpacing
                    }
                }

                QtControls.Label {
                    text: model.name

                    anchors {
                        left: icon.right
                        leftMargin: 8
                        verticalCenter: icon.verticalCenter
                        right: buttonConfigure.left
                    }
                }

                PlasmaComponents.Button {
                    id: buttonDelete

                    iconSource: "edit-delete"

                    anchors {
                        right: parent.right
                        rightMargin: units.smallSpacing
                        verticalCenter: icon.verticalCenter
                    }
                }

                PlasmaComponents.Button {
                    id: buttonConfigure

                    iconSource: "configure"

                    anchors {
                        right: buttonDelete.left
                        rightMargin: units.smallSpacing
                        verticalCenter: icon.verticalCenter
                    }
                }
            }
        }
    }
}
