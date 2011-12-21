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

#include "config-features.h"

#if !defined(HAVE_NEPOMUK) && !defined(Q_CC_MSVC)
    #warning "No Nepomuk, disabling some activity related features"
#endif

#ifdef HAVE_NEPOMUK

#include "NepomukActivityManager.h"

#include <KConfigGroup>

using namespace Nepomuk::Vocabulary;

NepomukActivityManager * NepomukActivityManager::s_instance = NULL;

NepomukActivityManager * NepomukActivityManager::self()
{
    if (!s_instance) {
        s_instance = new NepomukActivityManager();
    }

    return s_instance;
}

NepomukActivityManager::NepomukActivityManager()
{
}

NepomukActivityManager::~NepomukActivityManager()
{
}

// Methods

void NepomukActivityManager::init()
{
    Nepomuk::ResourceManager::instance()->init();
}

void NepomukActivityManager::syncActivities(const QStringList activityIds, KConfigGroup config, KConfigGroup iconsConfig)
{
    bool configNeedsSyncing = false;

    foreach (const QString & activityId, activityIds) {
        Nepomuk::Resource resource(activityId, KEXT::Activity());

        QString name = config.readEntry(activityId, QString());
        QString icon = iconsConfig.readEntry(activityId, QString());

        resource.setProperty(KEXT::activityIdentifier(), activityId);

        if (!name.isEmpty()) {
            resource.setLabel(name);
        }

        if (!icon.isEmpty()) {
            resource.setSymbols(QStringList() << icon);
        } else {
            QStringList symbols = resource.symbols();
            if (symbols.size() > 0) {
                iconsConfig.writeEntry(activityId, symbols.at(0));
                configNeedsSyncing = true;
            }
        }
    }

    if (configNeedsSyncing) config.sync();
}

void NepomukActivityManager::setActivityName(const QString & activity, const QString & name)
{
    activityResource(activity).setLabel(name);
}

void NepomukActivityManager::setActivityIcon(const QString & activity, const QString & icon)
{
    activityResource(activity).setSymbols(QStringList() << icon);
}


void NepomukActivityManager::setResourceMimeType(const KUrl & kuri, const QString & mimetype)
{
    Nepomuk::Resource resource(kuri);

    if (!resource.hasProperty(NIE::mimeType())) {
        resource.setProperty(NIE::mimeType(), mimetype);

        if (mimetype.startsWith("image/")) {
            resource.addType(NFO::Image());

        } else if (mimetype.startsWith("video/")) {
            resource.addType(NFO::Video());

        } else if (mimetype.startsWith("audio/")) {
            resource.addType(NFO::Audio());

        } else if (mimetype.startsWith("image/")) {
            resource.addType(NFO::Image());

        } else if (mimetype.startsWith("text/")) {
            resource.addType(NFO::TextDocument());

            if (mimetype == "text/plain") {
                resource.addType(NFO::PlainTextDocument());

            } else if (mimetype == "text/html") {
                    resource.addType(NFO::HtmlDocument());
            }
        }

    }

}

void NepomukActivityManager::setResourceTitle(const KUrl & kuri, const QString & title)
{
    Nepomuk::Resource resource(kuri);

    if (!kuri.isLocalFile() || !resource.hasProperty(NIE::title())) {
        resource.setProperty(NIE::title(), title);
    }
}

void NepomukActivityManager::linkResourceToActivity(const KUrl & resource, const QString & activity)
{
    // TODO:
    // I'd like a resource isRelated activity more than vice-versa
    // but the active models are checking for the other way round.
    // It is defined in the ontologies as a symmetric relation, but
    // Nepomuk doesn't care about that.

    // Nepomuk::Resource(KUrl(uri)).
    //     addIsRelated(d->activityResource(
    //         activity.isEmpty() ?
    //             CurrentActivity() : activity
    //         )
    //     );

    activityResource(activity).addIsRelated(Nepomuk::Resource(resource));
}

Nepomuk::Resource NepomukActivityManager::activityResource(const QString & id) const
{
    return Nepomuk::Resource(id, KEXT::Activity());
}

bool NepomukActivityManager::initialized() const
{
    return Nepomuk::ResourceManager::instance()->initialized();
}

void NepomukActivityManager::toRealUri(KUrl & kuri)
{
    if (kuri.scheme() == "nepomuk") {
        Nepomuk::Resource resource(kuri);

        if (resource.hasProperty(NIE::url())) {
            kuri = resource.property(NIE::url()).toUrl();
        }
    }
}

#endif // HAVE_NEPOMUK
