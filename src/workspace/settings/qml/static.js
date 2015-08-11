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

.pragma library

var currentlyOpenDialog = null

function dialogOpened(dialog)
{
    if (currentlyOpenDialog != null && currentlyOpenDialog != dialog) {
        currentlyOpenDialog.close();
    }

    currentlyOpenDialog = dialog;
}

function dialogClosed(dialog)
{
    if (currentlyOpenDialog == dialog) {
        currentlyOpenDialog = null;
    }
}

function closeAllDialogs()
{
    if (currentlyOpenDialog != null) {
        currentlyOpenDialog.close();
    }
}

function isAnyDialogShown()
{
    return currentlyOpenDialog != null;
}

function openActivityConfigurationDialog(
        dialogLoader,
        activityId,
        activityName,
        activityIcon,
        env
    )
{
  var open = function (item) {
        item.activityName = activityName;
        item.activityIconSource = activityIcon;

        env.kactivitiesExtras.getIsPrivate(activityId, function (isPrivate) {
            item.activityIsPrivate = isPrivate;
        });

        item.open();
    };

    if (dialogLoader.status != env.readyStatus) {

        dialogLoader.onLoaded.connect(function() {
            var item = dialogLoader.item;

            item.activityId = activityId;

            item.onAccepted.connect(function() {
                var id = item.activityId
                env.kactivities.setActivityName(id,
                    item.activityName,
                    function () {});
                env.kactivities.setActivityIcon(id,
                    item.activityIconSource,
                    function () {});
                env.kactivitiesExtras.setIsPrivate(id,
                    item.activityIsPrivate,
                    function () {});
            });

            open(item);

        });

        dialogLoader.source = Qt.resolvedUrl("ActivityConfigurationDialog.qml");

    } else {
        open(dialogLoader.item);

    }
}

function openActivityCreationDialog(
        dialogLoader,    // loader component that will contain the dialog
        env              // QML environment stuff that is invisible here
                         //   - kactivities - interface to KActivities
                         //   - readyStatus - Loader.Ready value
    )
{
    var open = function (item) {
        item.visible = true;
        item.opacity = 1;

        item.open(dialogLoader.height / 2);
    };

    if (dialogLoader.status != env.readyStatus) {

        dialogLoader.onLoaded.connect(function() {
            var item = dialogLoader.item;

            dialogLoader.item.accepted.connect(function() {
                env.kactivities.addActivity(item.activityName, function (id) {
                    env.kactivities.setActivityIcon(id,
                        dialogLoader.item.activityIconSource, function() {});
                    env.kactivitiesExtras.setIsPrivate(id,
                        item.activityIsPrivate, function () {});
                });
            });

            open(item);
        });

        dialogLoader.source = Qt.resolvedUrl("ActivityCreationDialog.qml");

    } else {
        open(dialogLoader.item);

    }

}

function openActivityDeletionDialog(
        dialogLoader,    // loader component that will contain the dialog
        activityId,      // id of the activity to be configured
        env              // QML environment stuff that is invisible here
                         //   - kactivities - interface to KActivities
                         //   - readyStatus - Loader.Ready value
    )
{
    var open = function (item) {
        item.open();
    };

    if (dialogLoader.status != env.readyStatus) {

        dialogLoader.onLoaded.connect(function() {
            var item = dialogLoader.item;

            item.activityId = activityId;

            item.onAccepted.connect(function() {
                var id = item.activityId
                env.kactivities.removeActivity(item.activityId, function () {})
            });

            open(item);

        });

        dialogLoader.source = Qt.resolvedUrl("ActivityDeletionDialog.qml");

    } else {
        open(dialogLoader.item);

    }

}

