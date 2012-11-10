/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef NEPOMUK_ACTIVITY_MANAGER_H
#define NEPOMUK_ACTIVITY_MANAGER_H

#include <config-features.h>

#ifdef HAVE_NEPOMUK
#    include <Nepomuk2/Resource>
#    define EXEC_NEPOMUK(A) NepomukActivityManager::self()->A
#    define NEPOMUK_PRESENT NepomukActivityManager::self()->initialized()
     namespace Nepomuk = Nepomuk2;
#else
#    define EXEC_NEPOMUK(A) // nepomuk disabled //
#    define NEPOMUK_PRESENT false
#endif

#include <QObject>

class Activities;

/**
 * Synchronizes KAMD data with Nepomuk
 * (activities, names, icons etc.)
 */
class NepomukActivityManager: public QObject {
    Q_OBJECT

#ifdef HAVE_NEPOMUK
private Q_SLOTS:
    void nepomukServiceStarted();
    void nepomukServiceStopped();
#endif

#ifdef HAVE_NEPOMUK
public:
    // TODO: Change from singleton to owned by Activities object,
    // with singleton accessor
    ~NepomukActivityManager();
    static NepomukActivityManager * self();

    // Returns whether nepomuk is present
    bool initialized() const;

    // Syncs the specified activities info between kamd and nepomuk storage
    void syncActivities(const QStringList & activityIds);

    // Sets the info for activity nepomuk resource
    void setActivityName(const QString & activity, const QString & name);
    void setActivityDescription(const QString & activity, const QString & description);
    void setActivityIcon(const QString & activity, const QString & icon);

    void setResourceMimeType(const KUrl & resource, const QString & mimetype);
    void setResourceTitle(const KUrl & resource, const QString & title);

    // Manages isRelated relations between various resources and activities
    void linkResourceToActivity(const KUrl & resource, const QString & activity);
    void unlinkResourceFromActivity(const KUrl & resource, const QString & activity);
    bool isResourceLinkedToActivity(const KUrl & resource, const QString & activity) const;
    QList < KUrl > resourcesLinkedToActivity(const QString & activity) const;

    // Gets a nepomuk:// url and returns a normal one, if exists
    void toRealUri(KUrl & url);

private:
    NepomukActivityManager();

    // Updates database to fit the latest version of
    // the kamd ontology
    void __updateOntology();

    // Returns activity nepomuk resource object
    Nepomuk::Resource activityResource(const QString & id) const;

    // Initializes the class by specifying activities manager
    void init(Activities * parent);

    bool m_nepomukPresent;
    static NepomukActivityManager * s_instance;
    QString m_currentActivity;

    // If sync activities failed due to nepomuk being started after us:
    QStringList m_cache_activityIds;

    // Ordinary pointer since this is our parent
    Activities * m_activities;
#endif // HAVE_NEPOMUK

public Q_SLOTS:
    void setCurrentActivity(const QString & id);
    void addActivity(const QString & activity);
    void removeActivity(const QString & activity);
    void reinit();

    friend class Activities;
};


#endif // NEPOMUK_ACTIVITY_MANAGER_H

