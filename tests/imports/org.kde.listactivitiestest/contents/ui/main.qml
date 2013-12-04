/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.activities 0.1 as Activities

ListView {
    id: main

    property int minimumWidth: 32
    property int minimumHeight: 32 // theme.mSize(theme.defaultFont).height * 14
    property int implicitWidth: minimumWidth * 1.5
    property int implicitHeight: minimumHeight * 1.5

    property int formFactor: plasmoid.formFactor



    model: modelMain

    Activities.ActivityModel {
        id: modelMain
    }

    add: Transition {
        NumberAnimation { properties: "x"; from: 300; duration: 1000 }
    }

    addDisplaced: Transition {
       NumberAnimation { properties: "x,y"; duration: 1000 }
    }

    remove: Transition {
        NumberAnimation { properties: "x"; to: 300; duration: 1000 }
    }

    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 1000 }
    }

    ListModel {
        id: modelDummy

        ListElement {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement {
            name: "Sam Wise"
            number: "555 0473"
        }
    }

    delegate: Column {
        height: 32
        Text {
            text: name
            height: 16
            font.bold: true
        }
        Text {
            text: "   id: " + id
            height: 16
        }

    }
}
