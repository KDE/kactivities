import QtQuick 2.0
Rectangle {
    color: "black"
    ListView {
        anchors.fill: parent
        anchors.margins: 8

        model: kamdmodel

        delegate: Item {
            height: 100
            width: 500

            Rectangle {
                id: titleRect
                height: 32

                color: '#202020'
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                Text {
                    text: model.title
                    anchors {
                        fill: parent
                        margins: 6
                    }
                    color: 'white'
                }

                Text {
                    text: "Score: " + model.score
                    anchors {
                        fill: parent
                        margins: 6
                    }
                    color: 'white'
                    horizontalAlignment: Text.AlignRight
                }
            }

            Rectangle {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: titleRect.bottom
                }

                color: '#303030'
                height: 64

                Text {
                    anchors {
                        fill: parent
                        margins: 6
                    }
                    color: 'white'
                    text: model.resource + "\n" + model.modified + "\n" + model.created
                }

            }
        }

        add: Transition {
            NumberAnimation { properties: "x,y"; from: 100; duration: 1000 }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }
        move: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }
        moveDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }
        // populate: Transition {
        //     NumberAnimation { properties: "x,y"; duration: 1000 }
        // }
        removeDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }
        remove: Transition {
            ParallelAnimation {
                NumberAnimation { property: "opacity"; to: 0; duration: 1000 }
                NumberAnimation { properties: "x,y"; to: 100; duration: 1000 }
            }
        }
    }
}
