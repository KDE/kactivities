/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (c) 2011 Sebastian Trueg <trueg@kde.org>
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

#include "NepomukPlugin.h"
#include "NepomukResourceScoreMaintainer.h"

#include "../../Event.h"
#include "kext.h"

#include <nepomuk/resource.h>
#include <nepomuk/nuao.h>
#include <nepomuk/resourcemanager.h>
#include <nepomuk/variant.h>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/NegationTerm>

#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <kdebug.h>

#include "NepomukCommon.h"
#include "Rankings.h"

using namespace Soprano::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace Nepomuk::Query;

NepomukPlugin * NepomukPlugin::s_instance = NULL;

NepomukPlugin::NepomukPlugin(QObject *parent, const QVariantList & args)
    : Plugin(parent)
{
    Q_UNUSED(args)
    s_instance = this;
}

bool NepomukPlugin::init()
{
    Rankings::init(this);

    connect(sharedInfo(), SIGNAL(currentActivityChanged(QString)),
            Rankings::self(), SLOT(setCurrentActivity(QString)));

    // TODO: Check for nepomuk and return false if not running
    return true;
}

NepomukPlugin * NepomukPlugin::self()
{
    return s_instance;
}

void NepomukPlugin::addEvents(const EventList & events)
{
    foreach (const Event& event, events) {
        kDebug() << "We are processing event" << event.type << event.uri;
        kDebug() << "for agent" << event.application << agentResource(event.application).resourceUri();

        switch (event.type) {
            case Event::Accessed:
            {
                // one-shot event

                Nepomuk::Resource eventRes = createDesktopEvent(event.uri, event.timestamp, event.application);
                eventRes.addType(NUAO::UsageEvent());
                eventRes.setProperty(NUAO::start(), event.timestamp);
                eventRes.setProperty(NUAO::end(), event.timestamp);

                kDebug() << "Created one-shot Accessed event" << eventRes;

                NepomukResourceScoreMaintainer::self()->processResource(event.uri, event.application);

                break;
            }

            case Event::Opened:
                // create a new event
                createDesktopEvent(event.uri, event.timestamp, event.application);

                break;

            case Event::Closed:
            {
                // We should find and close the last open event
                // TODO: This can have a problem if an app closes a document
                // while not in the current activity
                const QString query
                    = QString::fromLatin1(
                            "select ?r where { "
                                "?r a nuao:DesktopEvent . "
                                "?r %1 %2 . "
                                "?r %3 %4 . "
                                "?r %5 %6 . "
                                "?r nuao:start ?start . "
                                "OPTIONAL { ?r nuao:end ?d . } . "
                                "FILTER(!BOUND(?d)) . "
                            "}"
                            "ORDER BY DESC (?start) LIMIT 1"
                    ).arg(
                        /* %1 */ resN3(NUAO_targettedResource),
                        /* %2 */ resN3(anyResource(KUrl(event.uri))),
                        /* %3 */ resN3(NUAO_initiatingAgent),
                        /* %4 */ resN3(agentResource(event.application)),
                        /* %5 */ resN3(KExt::usedActivity()),
                        /* %6 */ resN3(currentActivityRes)
                    );
                kDebug() << query;

                Soprano::QueryResultIterator it
                        = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

                if (it.next()) {
                    kDebug() << "Closing the event";

                    Nepomuk::Resource eventRes(it[0].uri());
                    it.close();

                    eventRes.addProperty(NUAO::end(), event.timestamp);

                    NepomukResourceScoreMaintainer::self()->processResource(event.uri, event.application);
                }

                break;
            }

            case Event::UserEventType:
                NepomukResourceScoreMaintainer::self()->processResource(event.uri, event.application);
                break;

            default:
                // Nothing yet
                // TODO: Add focus and modification
                break;
        }


//        } else {
//            // find the corresponding event
//            // FIXME: enable this once the range of nao:identifier has been fixed and is no longer assumed to be rdfs:Resource
//            // resulting in a wrong query.
//            Query query(ResourceTypeTerm(NUAO::DesktopEvent())
//                        && ComparisonTerm(NUAO::involves(),
//                                          ResourceTerm(Nepomuk::Resource(KUrl(event.uri))), ComparisonTerm::Equal)
//                        && ComparisonTerm(NUAO::involves(),
//                                          ResourceTypeTerm(NAO::Agent())
//                                          && ComparisonTerm(NAO::identifier(), LiteralTerm(event.application), ComparisonTerm::Equal))
//                        && !ComparisonTerm(NUAO::end(), Term()));
//            query.setLimit(1);
//            query.setQueryFlags(Query::NoResultRestrictions);
//            const QString query = query.toSparqlQuery();
//
//            // TODO: Something strange is going on here - this should check for
//            // the activity as well
//            const QString query
//                    = QString::fromLatin1("select ?r where { "
//                                          "?r a nuao:DesktopEvent . "
//                                          "?r %1 %2 . "
//                                          "?r %3 %4 . "
//                                          "OPTIONAL { ?r nuao:end ?d . } . "
//                                          "FILTER(!BOUND(?d)) . } "
//                                          "LIMIT 1")
//                    .arg(
//                        /* %1 */ resN3(NUAO_targettedResource),
//                        /* %2 */ resN3(anyResource(KUrl(event.uri))),
//                        /* %3 */ resN3(NUAO_initiatingAgent),
//                        /* %4 */ resN3(agentResource(event.application))
//                    );
//
//            kDebug() << query;
//
//            Soprano::QueryResultIterator it
//                    = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);
//
//            if (it.next()) {
//                Nepomuk::Resource eventRes(it[0].uri());
//                it.close();
//
//                eventRes.addProperty(NUAO::end(), event.timestamp);
//                if (event.type == Event::Modified) {
//                    eventRes.addType(NUAO::ModificationEvent());
//                } else {
//                    eventRes.addType(NUAO::UsageEvent());
//                }
//
//                // TODO: We are not creating separate events for modifications
//                // // In case of a modification event we create a new event which will
//                // // be completed by the final Closed event since this one resource
//                // // modification is done now. It ended with saving the resource.
//                // if (event.type == Event::Modified) {
//                //     // create a new event
//                //     createDesktopEvent(event.uri, event.timestamp, event.application);
//                // }
//
//            } else {
//                kDebug() << "Failed to find matching Open event for resource" << event.uri << "and application" << event.application;
//            }
//
//            if (event.type == Event::Closed) {
//                NepomukResourceScoreMaintainer::self()->processResource(event.uri, event.application);
//            }
//        }
    }
}

Nepomuk::Resource NepomukPlugin::createDesktopEvent(const KUrl& uri, const QDateTime& startTime, const QString& app)
{
    kDebug() << "Creating a new desktop event" << uri << startTime << app;

    // one-shot event
    Nepomuk::Resource eventRes(QUrl(), NUAO::DesktopEvent());
    eventRes.addProperty(NUAO_targettedResource, anyResource(uri));
    eventRes.addProperty(NUAO::start(), startTime);

    kDebug() << "Created event" << eventRes.resourceUri()
             << "for resource" << ((Nepomuk::Resource(uri)).resourceUri());

    // the app
    Nepomuk::Resource appRes(app, NAO::Agent());
    eventRes.addProperty(NUAO_initiatingAgent, appRes);

    // the activity
    if (!m_currentActivity.isValid()
            || m_currentActivity.identifiers().isEmpty()
            || m_currentActivity.identifiers().first() != sharedInfo()->currentActivity()) {
        // update the current activity resource

        kDebug() << "Assigning the activity to the event";

        const QString query = QString::fromLatin1("select ?r where { "
                " ?r a %1 . "
                " ?r %2 %3 . "
                "} LIMIT 1"
            ).arg(
                /* %1 */ resN3(KExt::Activity()),
                /* %2 */ resN3(KExt::activityIdentifier()),
                /* %3 */ resN3(currentActivityRes)
            );

        Soprano::QueryResultIterator it = Nepomuk::ResourceManager::instance()->mainModel()
            ->executeQuery(query, Soprano::Query::QueryLanguageSparql);

        if (it.next()) {
            m_currentActivity = it[0].uri();
        } else {
            m_currentActivity = currentActivityRes;
            m_currentActivity.setProperty(KExt::activityIdentifier(), sharedInfo()->currentActivity());
        }
    }

    eventRes.setProperty(KExt::usedActivity(), m_currentActivity);

    return eventRes;
}

KAMD_EXPORT_PLUGIN(NepomukPlugin, "activitymanger_plugin_nepomuk")
