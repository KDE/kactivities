/*
 *   Copyright (C) 2011 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIVITIES_RESOURCEINSTANCE_H
#define ACTIVITIES_RESOURCEINSTANCE_H

#include <QObject>
#include <QUrl>

#include "kactivities_export.h"

namespace KActivities {

class ResourceInstancePrivate;

/**
 * This class is used to notify the system that a file, web page
 * or some other resource has been accessed.
 *
 * It provides methods to notify the system when the resource was
 * opened, modified and closed, along with in what window the
 * resource is shown.
 *
 * You should create an instance of this class for every resource
 * you open.
 *
 * "The system" in this case can be the backend for tracking
 * and automatically scoring files that are being accessed, the
 * system to show the open files per window in the taskbar,
 * the share-like-connect, etc.
 *
 * The user of this class shouldn't care about the backend
 * systems - everything is done under-the-hood automatically.
 *
 */
class KACTIVITIES_EXPORT ResourceInstance : public QObject {
    Q_OBJECT

    Q_PROPERTY(QUrl uri READ uri WRITE setUri)
    Q_PROPERTY(QString mimetype READ mimetype WRITE setMimetype)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(quintptr winId READ winId)

public:
    /**
     * Creates a new resource instance
     * @param wid id of the window that will show the resource
     * @param parent pointer to the parent object
     * @since 4.10
     */
    explicit ResourceInstance(quintptr wid, QObject *parent = Q_NULLPTR);

    /**
     * Creates a new resource instance
     * @param wid id of the window that will show the resource
     * @param application application's name (the name used for the .desktop file).
     *        If not specified, QCoreApplication::applicationName is used
     * @param parent pointer to the parent object
     */
    explicit ResourceInstance(quintptr wid, const QString &application, QObject *parent = Q_NULLPTR);

    /**
     * Creates a new resource instance and automatically
     * notifies the system that it was opened.
     *
     * In some special cases, where the URI of the resource is
     * being constantly changed (for example, in the world globe,
     * street map applications) you have two options:
     *  - to pass an empty resourceUri while passing the mimetype
     *  - to update the uri from time to time (in the example of
     *    the world map - to send URIs for major objects - cities
     *    or main streets.
     * and in both cases reimplementing the currentUri() method
     * which will return the exact URI shown at that specific moment.
     *
     * @param wid window id in which the resource is shown
     * @param resourceUri URI of the resource that is shown
     * @param mimetype the mime type of the resource
     * @param title the title of the resource
     * @param application application's name (the name used for the .desktop file).
     *        If not specified, QCoreApplication::applicationName is used
     * @param parent pointer to the parent object
     */
    ResourceInstance(quintptr wid, QUrl resourceUri, const QString &mimetype = QString(),
                     const QString &title = QString(), const QString &application = QString(),
                     QObject *parent = Q_NULLPTR);

    /**
     * Destroys the ResourceInstance and notifies the system
     * that the resource has been closed
     */
    ~ResourceInstance();

public Q_SLOTS:
    /**
     * Call this method to notify the system that you modified
     * (the contents of) the resource
     */
    void notifyModified();

    /**
     * Call this method to notify the system that the resource
     * has the focus in your application
     * @note You only need to call this in MDI applications
     */
    void notifyFocusedIn();

    /**
     * Call this method to notify the system that the resource
     * lost the focus in your application
     * @note You only need to call this in MDI applications
     */
    void notifyFocusedOut();

    /**
     * This is a convenience method that sets the new URI.
     * This is usually handled by sending the close event for
     * the previous URI, and an open event for the new one.
     */
    void setUri(const QUrl &newUri);

    /**
     * Sets the mimetype for this resource
     */
    void setMimetype(const QString &mimetype);

    /**
     * Sets the title for this resource
     */
    void setTitle(const QString &title);

Q_SIGNALS:
    /**
     * Emitted when the system wants to show the resource
     * represented by this ResourceInstance.
     *
     * You should listen to this signal if you have multiple
     * resources shown in one window (MDI). On catching it, show
     * the resource and give it focus.
     */
    void requestsFocus();

public:
    /**
     * @returns the current uri
     * The default implementation returns the URI that was passed
     * to the constructor.
     * You need to reimplement it only for the applications with
     * frequently updated URIs.
     */
    virtual QUrl uri() const;

    /**
     * @returns mimetype of the resource
     */
    QString mimetype() const;

    /**
     * @returns title of the resource
     */
    QString title() const;

    /**
     * @returns the window id
     */
    quintptr winId() const;

    /**
     * If there's no way to tell for how long an application is keeping
     * the resource open, you can just call this static method - it
     * will notify the system that the application has accessed the
     * resource
     * @param uri URI of the resource
     * @param application application's name (the name used for the .desktop file).
     *        If not specified, QCoreApplication::applicationName is used
     *
     */
    static void notifyAccessed(const QUrl &uri, const QString &application = QString());

private:
    const QScopedPointer<ResourceInstancePrivate> d;
};
}

#endif // ACTIVITIES_RESOURCEINSTANCE_H
