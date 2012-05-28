/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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
#    include <Nepomuk/ResourceManager>
#    include <Nepomuk/Resource>
#    include <Nepomuk/Variant>
#    include <Nepomuk/Vocabulary/NIE>
#    include <Nepomuk/Vocabulary/NFO>
#    include "kao.h"
#endif

#include <QObject>

class NepomukActivityManager: public QObject {
    Q_OBJECT

private Q_SLOTS:
    void nepomukServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner);

#ifdef HAVE_NEPOMUK
public:
    ~NepomukActivityManager();
    static NepomukActivityManager * self();

    void init();
    bool initialized() const;

    void syncActivities(const QStringList activityIds, KConfigGroup config, KConfigGroup iconsConfig);
    void setActivityName(const QString & activity, const QString & name);
    void setActivityDescription(const QString & activity, const QString & description);
    void setActivityIcon(const QString & activity, const QString & icon);

    void setResourceMimeType(const KUrl & resource, const QString & mimetype);
    void setResourceTitle(const KUrl & resource, const QString & title);

    void linkResourceToActivity(const KUrl & resource, const QString & activity);
    void unlinkResourceFromActivity(const KUrl & resource, const QString & activity);
    bool isResourceLinkedToActivity(const KUrl & resource, const QString & activity) const;
    QList < KUrl > resourcesLinkedToActivity(const QString & activity) const;

    void toRealUri(KUrl & url);

    void setCurrentActivity(const QString & id);

private:
    void __updateOntology();

    Nepomuk::Resource activityResource(const QString & id) const;

    NepomukActivityManager();

    bool m_nepomukPresent;
    static NepomukActivityManager * s_instance;
#endif // HAVE_NEPOMUK
};


#endif // NEPOMUK_ACTIVITY_MANAGER_H

