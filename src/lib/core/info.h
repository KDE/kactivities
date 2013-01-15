/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ACTIVITIES_INFO_H
#define ACTIVITIES_INFO_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QStringList>

#include <kurl.h>
#include <kdemacros.h>

#include "kactivities_export.h"

namespace KActivities {

class InfoPrivate;

/**
 * This class provides info about an activity. Most methods in it
 * require a Nepomuk backend running.
 *
 * This class is not thread-safe
 *
 * @see Consumer for info about activities
 *
 * The API of the class is synchronous, but the most used properties
 * are pre-fetched and cached. This means that, in order to get the least
 * amount of d-bus related locks, you should declare long-lived instances
 * of this class.
 *
 * For example, this is wrong (works, but blocks):
 * @code
 * void someMethod(const QString & activity) {
 *     Info info(activity);
 *     doSomethingWith(info.name());
 * }
 * @endcode
 *
 * The methods that are cached are marked as 'pre-fetched and cached'.
 * Methods that will block until the response from the service is returned
 * are marked as 'blocking'.
 *
 * @since 4.5
 */
class KACTIVITIES_EXPORT Info: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

public:
    explicit Info(const QString & activity, QObject * parent = 0 /*nullptr*/);
    ~Info();

    /**
     * @return true if the activity represented by this object exists and is valid
     */
    bool isValid() const;

    /**
     * Specifies which parts of this class are functional
     */
    enum Availability {
        Nothing = 0,             ///< No activity info provided (isValid is false)
        BasicInfo = 1,           ///< Basic info is provided
        Everything = 2           ///< Everything is available
    };

    /**
     * State of the activity
     */
    enum State {
        Invalid  = 0,
        Running  = 2,
        Starting = 3,
        Stopped  = 4,
        Stopping = 5
    };

    /**
     * @returns what info is provided by this instance of Info
     */
    Availability availability() const;

    /**
     * @returns the URI of this activity. The same URI is used by
     * activities KIO slave.
     */
    KUrl uri() const;

    /**
     * @deprecated we don't guarantee that nepomuk is the backend
     * @returns the Nepomuk resource URI of this activity
     * @note Functional only when availability is Everything
     */
    KDE_DEPRECATED
    KUrl resourceUri() const;

    /**
     * @returns the id of the activity
     */
    QString id() const;

    /**
     * @returns the name of the activity
     * @note Functional when availability is BasicInfo or Everything
     * @note This method is <b>pre-fetched and cached</b>
     */
    QString name() const;

    /**
     * @returns the icon of the activity. Icon can be a
     * freedesktop.org name or a file path. Or empty if
     * no icon is set.
     * @note Functional only when availability is Everything
     * @note This method is <b>pre-fetched and cached</b>
     */
    QString icon() const;

    /**
     * @returns the state of the activity
     * @note This method is <b>cached</b>
     */
    State state() const;

    /**
     * @returns true if encrypted
     * @since 4.8
     */
    KDE_DEPRECATED
    bool isEncrypted() const;

    /**
     * This function is provided for convenience.
     * @returns the name of the specified activity
     * @param id id of the activity
     * @note This method is <b>blocking</b>, you should use Info::name()
     */
    static QString name(const QString & id);

    /**
     * Links the specified resource to the activity
     * @param resourceUri resource URI
     * @note This method is <b>asynchronous</b>
     */
    void linkResource(const KUrl & resourceUri);


    /**
     * Unlinks the specified resource from the activity
     * @param resourceUri resource URI
     * @note This method is <b>asynchronous</b>
     */
    void unlinkResource(const KUrl & resourceUri);


    /**
     * @returns the list of linked resources
     * @note This method is <b>blocking</b>
     */
    KDE_DEPRECATED
    KUrl::List linkedResources() const;


    /**
     * @returns whether a resource is linked to this activity
     * @note this method is <b>blocking</b>
     * @since 4.11
     */
    bool isResourceLinked(const KUrl & resourceUri);

Q_SIGNALS:
    /**
     * Emitted when the activity's name, icon or some custom property is changed
     */
    void infoChanged();

    /**
     * Emitted when the name is changed
     */
    void nameChanged(const QString & name);

    /**
     * Emitted when the icon was changed
     */
    void iconChanged(const QString & icon);

    /**
     * Emitted when the activity is added
     */
    void added();

    /**
     * Emitted when the activity is removed
     */
    void removed();

    /**
     * Emitted when the activity is started
     */
    void started();

    /**
     * Emitted when the activity is stopped
     */
    void stopped();

    /**
     * Emitted when the activity changes state
     * @param state new state of the activity
     */
    void stateChanged(KActivities::Info::State state);

private:
    InfoPrivate * const d;

    Q_PRIVATE_SLOT(d, void activityStateChanged(const QString &, int))
    Q_PRIVATE_SLOT(d, void added(const QString &))
    Q_PRIVATE_SLOT(d, void removed(const QString &))
    Q_PRIVATE_SLOT(d, void started(const QString &))
    Q_PRIVATE_SLOT(d, void stopped(const QString &))
    Q_PRIVATE_SLOT(d, void infoChanged(const QString &))
    Q_PRIVATE_SLOT(d, void nameChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void iconChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void setServicePresent(bool))
    Q_PRIVATE_SLOT(d, void nameCallFinished(QDBusPendingCallWatcher*))
    Q_PRIVATE_SLOT(d, void iconCallFinished(QDBusPendingCallWatcher*))

    friend class InfoPrivate;
};

} // namespace KActivities

#endif // ACTIVITIES_INFO_H

