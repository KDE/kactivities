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

#include <config-features.h>

#if !defined(HAVE_NEPOMUK) && defined(__GNUC__) // krazy:skip
    #warning "No Nepomuk, disabling some activity related features"
#endif

#include "NepomukActivityManager.h"

#ifdef HAVE_NEPOMUK

#include "kao.h"
#include <KConfigGroup>
#include <KDebug>

#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk2/Variant>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NFO>

#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <Activities.h>

// TODO: Move into a plugin
// The activities KIO works only if nepomuk is present, so we can
// freely send the change event here
#include <KDirNotify>

#include <utils/nullptr.h>
#include <utils/val.h>

namespace Nepomuk = Nepomuk2;
using namespace Nepomuk::Vocabulary;
using namespace KDE::Vocabulary;
using namespace Soprano::Vocabulary;

NepomukActivityManager * NepomukActivityManager::s_instance = nullptr;

NepomukActivityManager * NepomukActivityManager::self()
{
    if (!s_instance) {
        s_instance = new NepomukActivityManager();
    }

    return s_instance;
}

NepomukActivityManager::NepomukActivityManager()
    : m_nepomukPresent(false), m_activities(nullptr)
{
    connect(Nepomuk::ResourceManager::instance(), SIGNAL(nepomukSystemStarted()), this, SLOT(nepomukServiceStarted()));
    connect(Nepomuk::ResourceManager::instance(), SIGNAL(nepomukSystemStopped()), this, SLOT(nepomukServiceStopped()));

    if (Nepomuk::ResourceManager::instance()->initialized()) {
        nepomukServiceStarted();
    }
}

NepomukActivityManager::~NepomukActivityManager()
{
    s_instance = nullptr;
}

// Methods

void NepomukActivityManager::init(Activities * parent)
{
    if (parent && !m_activities) {
        m_activities = parent;
        setParent(parent);

        // Giving the time to Activities object to properly form
        QMetaObject::invokeMethod(this, "reinit", Qt::QueuedConnection);
    }
}

void NepomukActivityManager::reinit()
{
    val activitiesList = m_activities->ListActivities();
    static val prefix = QLatin1String("activities:/");

    syncActivities(activitiesList);

    if (!m_nepomukPresent) {
        // If nepomuk is offline, removes items from activities kio

        QStringList removed;

        removed << prefix + "current";
        foreach (const QString & activity, activitiesList) {
            removed << prefix + activity;
        }

        org::kde::KDirNotify::emitFilesRemoved(removed);
    }
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
    if (!m_nepomukPresent) return;

    // run once guard
    static bool init = false;
    if (init) return;
    init = true;

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


void NepomukActivityManager::syncActivities(const QStringList & activityIds)
{
    if (!m_nepomukPresent || !m_activities) {
        m_cache_activityIds.append(activityIds);
        return;
    }

    // Before we start syncing, we need to convert KEXT -> KAO
    // TODO: Remove after 4.10
    // __updateOntology();

    foreach (const QString & activityId, activityIds) {
        org::kde::KDirNotify::emitFilesAdded("activities:/" + activityId);

        auto resource = activityResource(activityId);

        val name = m_activities->ActivityName(activityId);
        val icon = m_activities->ActivityIcon(activityId);

        resource.setProperty(KAO::activityIdentifier(), activityId);

        if (!name.isEmpty()) {
            resource.setLabel(name);
        }

        if (!icon.isEmpty()) {
            resource.setSymbols(QStringList() << icon);
        } else {
            QStringList symbols = resource.symbols();
            if (symbols.size() > 0) {
                m_activities->SetActivityIcon(activityId, symbols.at(0));
            }
        }
    }

    org::kde::KDirNotify::emitFilesAdded("activities:/current");

    m_cache_activityIds.clear();
}

void NepomukActivityManager::setActivityName(const QString & activity, const QString & name)
{
    if (!m_nepomukPresent) {
        m_cache_activityIds << activity;
        return;
    }

    activityResource(activity).setLabel(name);
}

void NepomukActivityManager::setActivityIcon(const QString & activity, const QString & icon)
{
    if (!m_nepomukPresent) {
        m_cache_activityIds << activity;
        return;
    }

    activityResource(activity).setSymbols(QStringList() << icon);
}


void NepomukActivityManager::setResourceMimeType(const KUrl & kuri, const QString & mimetype)
{
    if (!m_nepomukPresent) return;

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
    if (!m_nepomukPresent) return;

    Nepomuk::Resource resource(kuri);

    if (!kuri.isLocalFile() || !resource.hasProperty(NIE::title())) {
        resource.setProperty(NIE::title(), title);
    }
}

void NepomukActivityManager::linkResourceToActivity(const KUrl & resource, const QString & activity)
{
    if (!m_nepomukPresent) return;

    activityResource(activity).addIsRelated(Nepomuk::Resource(resource));

    if (m_currentActivity == activity)
        org::kde::KDirNotify::emitFilesAdded("activities:/current");
    org::kde::KDirNotify::emitFilesAdded("activities:/" + activity);
}

void NepomukActivityManager::unlinkResourceFromActivity(const KUrl & resource, const QString & activity)
{
    if (!m_nepomukPresent) return;

    activityResource(activity).removeProperty(NAO::isRelated(), Nepomuk::Resource(resource));

    if (m_currentActivity == activity)
        org::kde::KDirNotify::emitFilesAdded("activities:/current");
    org::kde::KDirNotify::emitFilesAdded("activities:/" + activity);
}

QList <KUrl> NepomukActivityManager::resourcesLinkedToActivity(const QString & activity) const
{
    QList <KUrl> result;

    if (!m_nepomukPresent) return result;

    foreach (const Nepomuk::Resource & resource, activityResource(activity).isRelateds()) {
        if (resource.hasProperty(NIE::url())) {
            result << resource.property(NIE::url()).toUrl();
        } else {
            result << resource.uri();
        }
    }

    return result;
}

bool NepomukActivityManager::isResourceLinkedToActivity(const KUrl & resource, const QString & activity) const
{
    if (!m_nepomukPresent) return false;

    return resourcesLinkedToActivity(activity).contains(resource);
}

Nepomuk::Resource NepomukActivityManager::activityResource(const QString & id) const
{
    if (!m_nepomukPresent) return Nepomuk::Resource();

    Nepomuk::Resource resource(id, KAO::Activity());

    return resource;
}

bool NepomukActivityManager::initialized() const
{
    return m_nepomukPresent;
}

void NepomukActivityManager::toRealUri(KUrl & kuri)
{
    if (!m_nepomukPresent) return;

    if (kuri.scheme() == "nepomuk") {
        Nepomuk::Resource resource(kuri);

        if (resource.hasProperty(NIE::url())) {
            kuri = resource.property(NIE::url()).toUrl();
        }
    }
}

#endif // HAVE_NEPOMUK

#ifdef HAVE_NEPOMUK

void NepomukActivityManager::nepomukServiceStarted()
{
    kDebug() << "Nepomuk was started";
    m_nepomukPresent = true;
    init(nullptr);

    if (!m_cache_activityIds.isEmpty()) {
        syncActivities(m_cache_activityIds);
    }

}

void NepomukActivityManager::nepomukServiceStopped()
{
    kDebug() << "Nepomuk was stopped";
    m_nepomukPresent = false;
}

void NepomukActivityManager::setCurrentActivity(const QString & id)
{
    m_currentActivity = id;

    org::kde::KDirNotify::emitFilesAdded("activities:/current");
}

void NepomukActivityManager::addActivity(const QString & activity)
{
    syncActivities(QStringList() << activity);

    org::kde::KDirNotify::emitFilesAdded("activities:/");
    org::kde::KDirNotify::emitFilesAdded("activities:/" + activity);
}

void NepomukActivityManager::removeActivity(const QString & activity)
{
    Q_UNUSED(activity)
    org::kde::KDirNotify::emitFilesAdded("activities:/");
}

#else // doesn't HAVE_NEPOMUK

void NepomukActivityManager::setCurrentActivity(const QString & activity)
{
    Q_UNUSED(activity)
}

void NepomukActivityManager::addActivity(const QString & activity)
{
    Q_UNUSED(activity)
}

void NepomukActivityManager::removeActivity(const QString & activity)
{
    Q_UNUSED(activity)
}

void NepomukActivityManager::reinit()
{
}

#endif // HAVE_NEPOMUK

