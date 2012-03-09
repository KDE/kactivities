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

#include "kao.h"
#include <KConfigGroup>
#include <Soprano/Vocabulary/NAO>

#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>


using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;

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


// Before we start syncing, we need to convert KEXT -> KAO ////////////////////////////
#define rdf_type                QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#type")
#define kext_Activity           QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#Activity")
#define kext_ResourceScoreCache QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#ResourceScoreCache")
#define kext_activityIdentifier QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#activityIdentifier")
#define kext_targettedResource  QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#targettedResource")
#define kext_initiatingAgent    QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#initiatingAgent")
#define kext_cachedScore        QUrl("http://nepomuk.kde.org/ontologies/2010/11/29/kext#cachedScore")


void NepomukActivityManager::__updateOntology()
{
    KConfig config("activitymanagerrc");
    KConfigGroup configGroup(&config, "main");

    int version = configGroup.readEntry("ontologyVersion", 0);

    if (version < 1) {
        // First, let us block this function for being invoked again later
        configGroup.writeEntry("ontologyVersion", 1);
        configGroup.sync();

        // Convert all the activities
        const QString query = QString::fromLatin1("select ?resource where { ?resource a %1 . }");

        Soprano::QueryResultIterator it
            = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
                query.arg(Soprano::Node::resourceToN3(kext_Activity)), Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            Nepomuk::Resource result(it[0].uri());

            result.addType(KAO::Activity());
            result.removeProperty(rdf_type, kext_Activity);

            result.setProperty(KAO::activityIdentifier(), result.property(kext_activityIdentifier));
            result.removeProperty(kext_activityIdentifier);
        }

        it.close();

        // Convert all the score caches
        it = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
                query.arg(Soprano::Node::resourceToN3(kext_ResourceScoreCache)), Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            Nepomuk::Resource result(it[0].uri());

            result.addType(KAO::ResourceScoreCache());
            result.removeProperty(rdf_type, kext_ResourceScoreCache);

            result.setProperty(KAO::targettedResource(), result.property(kext_targettedResource));
            result.removeProperty(kext_targettedResource);

            result.setProperty(KAO::initiatingAgent(), result.property(kext_initiatingAgent));
            result.removeProperty(kext_initiatingAgent);

            result.setProperty(KAO::cachedScore(), result.property(kext_cachedScore));
            result.removeProperty(kext_cachedScore);
        }

        it.close();
    }
}
///////////////////////////////////////////////////////////////////////////////////////


void NepomukActivityManager::syncActivities(const QStringList activityIds, KConfigGroup config, KConfigGroup iconsConfig)
{
    // Before we start syncing, we need to convert KEXT -> KAO
    __updateOntology();

    bool configNeedsSyncing = false;

    foreach (const QString & activityId, activityIds) {
        Nepomuk::Resource resource(activityId, KAO::Activity());

        QString name = config.readEntry(activityId, QString());
        QString icon = iconsConfig.readEntry(activityId, QString());

        resource.setProperty(KAO::activityIdentifier(), activityId);

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

void NepomukActivityManager::unlinkResourceToActivity(const KUrl & resource, const QString & activity)
{
    activityResource(activity).removeProperty(NAO::isRelated(), Nepomuk::Resource(resource));
}

QList <KUrl> NepomukActivityManager::resourcesLinkedToActivity(const QString & activity) const
{
    QList <KUrl> result;

    foreach (const Nepomuk::Resource & resource, activityResource(activity).isRelateds()) {
        if (resource.hasProperty(NIE::url())) {
            result << resource.property(NIE::url()).toUrl();
        } else {
            result << resource.resourceUri();
        }
    }

    return result;
}

Nepomuk::Resource NepomukActivityManager::activityResource(const QString & id) const
{
    return Nepomuk::Resource(id, KAO::Activity());
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
