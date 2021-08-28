/*
    SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Self
#include "resourcemodel.h"

// Qt
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QModelIndex>
#include <QSqlQuery>
#include <QUuid>

// KDE
#include <kconfig.h>
#include <kdesktopfile.h>
#include <kfileitem.h>
#include <ksharedconfig.h>

// STL and Boost
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/numeric.hpp>
#include <mutex>

// Local
#include "common/dbus/common.h"
#include "utils/dbusfuture_p.h"
#include "utils/range.h"

#define ENABLE_QJSVALUE_CONTINUATION
#include "utils/continue_with.h"

#define ACTIVITY_COLUMN 0
#define AGENT_COLUMN 1
#define RESOURCE_COLUMN 2
#define UNKNOWN_COLUMN 3

using kamd::utils::continue_with;

namespace KActivities
{
namespace Imports
{
class ResourceModel::LinkerService : public QDBusInterface
{
private:
    LinkerService()
        : KAMD_DBUS_INTERFACE("Resources/Linking", ResourcesLinking, nullptr)
    {
    }

public:
    static std::shared_ptr<LinkerService> self()
    {
        static std::weak_ptr<LinkerService> s_instance;
        static std::mutex singleton;

        std::lock_guard<std::mutex> singleton_lock(singleton);

        auto result = s_instance.lock();

        if (s_instance.expired()) {
            result.reset(new LinkerService());
            s_instance = result;
        }

        return result;
    }
};

ResourceModel::ResourceModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_shownActivities(QStringLiteral(":current"))
    , m_shownAgents(QStringLiteral(":current"))
    , m_defaultItemsLoaded(false)
    , m_linker(LinkerService::self())
    , m_config(KSharedConfig::openConfig("kactivitymanagerd-resourcelinkingrc")->group("Order"))
{
    // NOTE: What to do if the file does not exist?
    //       Ignoring that case since the daemon creates it on startup.
    //       Is it plausible that somebody will instantiate the ResourceModel
    //       before the daemon is started?

    const QString databaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/");

    m_databaseFile = databaseDir + QStringLiteral("database");

    loadDatabase();

    connect(&m_service, &KActivities::Consumer::currentActivityChanged, this, &ResourceModel::onCurrentActivityChanged);

    connect(m_linker.get(), SIGNAL(ResourceLinkedToActivity(QString, QString, QString)), this, SLOT(onResourceLinkedToActivity(QString, QString, QString)));
    connect(m_linker.get(),
            SIGNAL(ResourceUnlinkedFromActivity(QString, QString, QString)),
            this,
            SLOT(onResourceUnlinkedFromActivity(QString, QString, QString)));

    setDynamicSortFilter(true);
    sort(0);
}

bool ResourceModel::loadDatabase()
{
    if (m_database.isValid())
        return true;
    if (!QFile(m_databaseFile).exists())
        return false;

    // TODO: Database connection naming could be smarter (thread-id-based,
    //       reusing connections...?)
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("kactivities_db_resources_") + QString::number((quintptr)this));

    // qDebug() << "Database file is: " << m_databaseFile;
    m_database.setDatabaseName(m_databaseFile);

    m_database.open();

    m_databaseModel = new QSqlTableModel(this, m_database);
    m_databaseModel->setTable("ResourceLink");
    m_databaseModel->select();

    setSourceModel(m_databaseModel);

    reloadData();

    return true;
}

ResourceModel::~ResourceModel()
{
}

QVariant ResourceModel::dataForColumn(const QModelIndex &index, int column) const
{
    if (!m_database.isValid())
        return QVariant();

    return m_databaseModel->data(index.sibling(index.row(), column), Qt::DisplayRole);
}

bool ResourceModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftResource = dataForColumn(left, RESOURCE_COLUMN).toString();
    const auto rightResource = dataForColumn(right, RESOURCE_COLUMN).toString();

    const bool hasLeft = m_sorting.contains(leftResource);
    const bool hasRight = m_sorting.contains(rightResource);

    return (hasLeft && !hasRight) ? true
        : (!hasLeft && hasRight)  ? false
        : (hasLeft && hasRight)   ? m_sorting.indexOf(leftResource) < m_sorting.indexOf(rightResource)
                                  : QString::compare(leftResource, rightResource, Qt::CaseInsensitive) < 0;
}

QHash<int, QByteArray> ResourceModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"},
            {Qt::DecorationRole, "decoration"},
            {ResourceRole, "uri"},
            {AgentRole, "agent"},
            {ActivityRole, "activity"},
            {DescriptionRole, "subtitle"}};
}

template<typename Validator>
inline QStringList validateList(const QString &values, Validator validator)
{
    using boost::adaptors::filtered;
    using kamd::utils::as_collection;

    auto result = as_collection<QStringList>(values.split(',') | filtered(validator));

    if (result.isEmpty()) {
        result.append(QStringLiteral(":current"));
    }

    return result;
}

void ResourceModel::setShownActivities(const QString &activities)
{
    m_shownActivities = validateList(activities, [&](const QString &activity) {
        return activity == ":current" || activity == ":any" || activity == ":global" || !QUuid(activity).isNull();
    });

    reloadData();
    Q_EMIT shownActivitiesChanged();
}

void ResourceModel::setShownAgents(const QString &agents)
{
    m_shownAgents = validateList(agents, [&](const QString &agent) {
        return agent == ":current" || agent == ":any" || agent == ":global" || (!agent.isEmpty() && !agent.contains('\'') && !agent.contains('"'));
    });

    loadDefaultsIfNeeded();
    reloadData();
    Q_EMIT shownAgentsChanged();
}

QString ResourceModel::shownActivities() const
{
    return m_shownActivities.join(',');
}

QString ResourceModel::shownAgents() const
{
    return m_shownAgents.join(',');
}

QString ResourceModel::defaultItemsConfig() const
{
    return m_defaultItemsConfig;
}

void ResourceModel::setDefaultItemsConfig(const QString &defaultItemsConfig)
{
    m_defaultItemsConfig = defaultItemsConfig;
    loadDefaultsIfNeeded();
}

QString ResourceModel::activityToWhereClause(const QString &shownActivity) const
{
    return QStringLiteral(" OR usedActivity=")
        + (shownActivity == ":current"      ? "'" + m_service.currentActivity() + "'"
               : shownActivity == ":any"    ? "usedActivity"
               : shownActivity == ":global" ? "''"
                                            : "'" + shownActivity + "'");
}

QString ResourceModel::agentToWhereClause(const QString &shownAgent) const
{
    return QStringLiteral(" OR initiatingAgent=")
        + (shownAgent == ":current"      ? "'" + QCoreApplication::applicationName() + "'"
               : shownAgent == ":any"    ? "initiatingAgent"
               : shownAgent == ":global" ? "''"
                                         : "'" + shownAgent + "'");
}

QString ResourceModel::whereClause(const QStringList &activities, const QStringList &agents) const
{
    using boost::accumulate;
    using namespace kamd::utils;

    // qDebug() << "Getting the where clause for: " << activities << " " << agents;

    // Defining the transformation functions for generating the SQL WHERE clause
    // from the specified activity/agent. They also resolve the special values
    // like :current, :any and :global.

    auto activityToWhereClause = transformed(&ResourceModel::activityToWhereClause, this);
    auto agentToWhereClause = transformed(&ResourceModel::agentToWhereClause, this);

    // Generating the SQL WHERE part by concatenating the generated clauses.
    // The generated query will be in the form of '0 OR clause1 OR clause2 ...'

    const QString whereActivity = accumulate(activities | activityToWhereClause, QStringLiteral("0"));

    const QString whereAgent = accumulate(agents | agentToWhereClause, QStringLiteral("0"));

    // qDebug() << "This is the filter: " << '(' + whereActivity + ") AND (" + whereAgent + ')';

    return '(' + whereActivity + ") AND (" + whereAgent + ')';
}

void ResourceModel::reloadData()
{
    m_sorting = m_config.readEntry(m_shownAgents.first(), QStringList());

    if (!m_database.isValid())
        return;
    m_databaseModel->setFilter(whereClause(m_shownActivities, m_shownAgents));
}

void ResourceModel::onCurrentActivityChanged(const QString &activity)
{
    Q_UNUSED(activity);

    if (m_shownActivities.contains(":current")) {
        reloadData();
    }
}

QVariant ResourceModel::data(const QModelIndex &proxyIndex, int role) const
{
    auto index = mapToSource(proxyIndex);

    if (role == Qt::DisplayRole || role == DescriptionRole || role == Qt::DecorationRole) {
        auto uri = dataForColumn(index, RESOURCE_COLUMN).toString();

        // TODO: Will probably need some more special handling -
        //       for application:/ and a few more

        if (uri.startsWith('/')) {
            uri = QLatin1String("file://") + uri;
        }

        KFileItem file(uri);
        // clang-format off
        if (file.mimetype() == "application/x-desktop") {
            KDesktopFile desktop(file.localPath());

            return role == Qt::DisplayRole    ? desktop.readGenericName() :
                   role == DescriptionRole    ? desktop.readName() :
                   role == Qt::DecorationRole ? desktop.readIcon() : QVariant();
        }

        return role == Qt::DisplayRole    ? file.name() :
               role == Qt::DecorationRole ? file.iconName() : QVariant();
    }

    return dataForColumn(index,
            role == ResourceRole ? RESOURCE_COLUMN :
            role == AgentRole    ? AGENT_COLUMN :
            role == ActivityRole ? ACTIVITY_COLUMN :
                                   UNKNOWN_COLUMN
        );
    // clang-format on
}

void ResourceModel::linkResourceToActivity(const QString &resource, const QJSValue &callback) const
{
    linkResourceToActivity(resource, m_shownActivities.first(), callback);
}

void ResourceModel::linkResourceToActivity(const QString &resource, const QString &activity, const QJSValue &callback) const
{
    linkResourceToActivity(m_shownAgents.first(), resource, activity, callback);
}

void ResourceModel::linkResourceToActivity(const QString &agent, const QString &_resource, const QString &activity, const QJSValue &callback) const
{
    if (activity == ":any") {
        qWarning() << ":any is not a valid activity specification for linking";
        return;
    }

    auto resource = validateResource(_resource);

    // qDebug() << "ResourceModel: Linking resource to activity: --------------------------------------------------\n"
    //          << "ResourceModel:         Resource: " << resource << "\n"
    //          << "ResourceModel:         Agents: " << agent << "\n"
    //          << "ResourceModel:         Activities: " << activity << "\n";

    kamd::utils::continue_with(DBusFuture::asyncCall<void>(m_linker.get(),
                                                           QStringLiteral("LinkResourceToActivity"),
                                                           agent,
                                                           resource,
                                                           activity == ":current"      ? m_service.currentActivity()
                                                               : activity == ":global" ? ""
                                                                                       : activity),
                               callback);
}

void ResourceModel::unlinkResourceFromActivity(const QString &resource, const QJSValue &callback)
{
    unlinkResourceFromActivity(m_shownAgents, resource, m_shownActivities, callback);
}

void ResourceModel::unlinkResourceFromActivity(const QString &resource, const QString &activity, const QJSValue &callback)
{
    unlinkResourceFromActivity(m_shownAgents, resource, QStringList() << activity, callback);
}

void ResourceModel::unlinkResourceFromActivity(const QString &agent, const QString &resource, const QString &activity, const QJSValue &callback)
{
    unlinkResourceFromActivity(QStringList() << agent, resource, QStringList() << activity, callback);
}

void ResourceModel::unlinkResourceFromActivity(const QStringList &agents, const QString &_resource, const QStringList &activities, const QJSValue &callback)
{
    auto resource = validateResource(_resource);

    // qDebug() << "ResourceModel: Unlinking resource from activity: ----------------------------------------------\n"
    //          << "ResourceModel:         Resource: " << resource << "\n"
    //          << "ResourceModel:         Agents: " << agents << "\n"
    //          << "ResourceModel:         Activities: " << activities << "\n";

    for (const auto &agent : agents) {
        for (const auto &activity : activities) {
            if (activity == ":any") {
                qWarning() << ":any is not a valid activity specification for linking";
                return;
            }

            // We might want to compose the continuations into one
            // so that the callback gets called only once,
            // but we don't care about that at the moment
            kamd::utils::continue_with(DBusFuture::asyncCall<void>(m_linker.get(),
                                                                   QStringLiteral("UnlinkResourceFromActivity"),
                                                                   agent,
                                                                   resource,
                                                                   activity == ":current"      ? m_service.currentActivity()
                                                                       : activity == ":global" ? ""
                                                                                               : activity),
                                       callback);
        }
    }
}

bool ResourceModel::isResourceLinkedToActivity(const QString &resource)
{
    return isResourceLinkedToActivity(m_shownAgents, resource, m_shownActivities);
}

bool ResourceModel::isResourceLinkedToActivity(const QString &resource, const QString &activity)
{
    return isResourceLinkedToActivity(m_shownAgents, resource, QStringList() << activity);
}

bool ResourceModel::isResourceLinkedToActivity(const QString &agent, const QString &resource, const QString &activity)
{
    return isResourceLinkedToActivity(QStringList() << agent, resource, QStringList() << activity);
}

bool ResourceModel::isResourceLinkedToActivity(const QStringList &agents, const QString &_resource, const QStringList &activities)
{
    if (!m_database.isValid())
        return false;

    auto resource = validateResource(_resource);

    // qDebug() << "ResourceModel: Testing whether the resource is linked to activity: ----------------------------\n"
    //          << "ResourceModel:         Resource: " << resource << "\n"
    //          << "ResourceModel:         Agents: " << agents << "\n"
    //          << "ResourceModel:         Activities: " << activities << "\n";

    QSqlQuery query(m_database);
    query.prepare(
        "SELECT targettedResource "
        "FROM ResourceLink "
        "WHERE targettedResource=:resource AND "
        + whereClause(activities, agents));
    query.bindValue(":resource", resource);
    query.exec();

    auto result = query.next();

    // qDebug() << "Query: " << query.lastQuery();
    //
    // if (query.lastError().isValid()) {
    //     qDebug() << "Error: " << query.lastError();
    // }
    //
    // qDebug() << "Result: " << result;

    return result;
}

void ResourceModel::onResourceLinkedToActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity)
{
    Q_UNUSED(targettedResource);

    if (!loadDatabase())
        return;

    auto matchingActivity = boost::find_if(m_shownActivities, [&](const QString &shownActivity) {
        return
            // If the activity is not important
            shownActivity == ":any" ||
            // or we are listening for the changes for the current activity
            (shownActivity == ":current" && usedActivity == m_service.currentActivity()) ||
            // or we want the globally linked resources
            (shownActivity == ":global" && usedActivity.isEmpty()) ||
            // or we have a specific activity in mind
            shownActivity == usedActivity;
    });

    auto matchingAgent = boost::find_if(m_shownAgents, [&](const QString &shownAgent) {
        return
            // If the agent is not important
            shownAgent == ":any" ||
            // or we are listening for the changes for the current agent
            (shownAgent == ":current" && initiatingAgent == QCoreApplication::applicationName()) ||
            // or for links that are global, and not related to a specific agent
            (shownAgent == ":global" && initiatingAgent.isEmpty()) ||
            // or we have a specific agent to listen for
            shownAgent == initiatingAgent;
    });

    if (matchingActivity != m_shownActivities.end() && matchingAgent != m_shownAgents.end()) {
        // TODO: This might be smarter possibly, but might collide
        //       with the SQL model. Implement a custom model with internal
        //       cache instead of basing it on QSqlModel.
        reloadData();
    }
}

void ResourceModel::onResourceUnlinkedFromActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity)
{
    // These are the same at the moment
    onResourceLinkedToActivity(initiatingAgent, targettedResource, usedActivity);
}

void ResourceModel::setOrder(const QStringList &resources)
{
    m_sorting = resources;
    m_config.writeEntry(m_shownAgents.first(), m_sorting);
    m_config.sync();
    invalidate();
}

void ResourceModel::move(int sourceItem, int destinationItem)
{
    QStringList resources;
    const int rows = rowCount();

    for (int row = 0; row < rows; row++) {
        resources << resourceAt(row);
    }

    if (sourceItem < 0 || sourceItem >= rows || destinationItem < 0 || destinationItem >= rows) {
        return;
    }

    // Moving one item from the source item's location to the location
    // after the destination item
    std::rotate(resources.begin() + sourceItem, resources.begin() + sourceItem + 1, resources.begin() + destinationItem + 1);

    setOrder(resources);
}

void ResourceModel::sortItems(Qt::SortOrder sortOrder)
{
    typedef QPair<QString, QString> Resource;
    QList<Resource> resources;
    const int rows = rowCount();

    for (int row = 0; row < rows; ++row) {
        resources << qMakePair(resourceAt(row), displayAt(row));
    }

    std::sort(resources.begin(), resources.end(), [sortOrder](const Resource &left, const Resource &right) {
        return sortOrder == Qt::AscendingOrder ? left.second < right.second : right.second < left.second;
    });

    QStringList result;

    for (const auto &resource : std::as_const(resources)) {
        result << resource.first;
    }

    setOrder(result);
}

KConfigGroup ResourceModel::config() const
{
    return KSharedConfig::openConfig("kactivitymanagerd-resourcelinkingrc")->group("Order");
}

int ResourceModel::count() const
{
    return QSortFilterProxyModel::rowCount();
}

QString ResourceModel::displayAt(int row) const
{
    return data(index(row, 0), Qt::DisplayRole).toString();
}

QString ResourceModel::resourceAt(int row) const
{
    return validateResource(data(index(row, 0), ResourceRole).toString());
}

void ResourceModel::loadDefaultsIfNeeded() const
{
    // Did we get a request to actually do anything?
    if (m_defaultItemsConfig.isEmpty())
        return;
    if (m_shownAgents.size() == 0)
        return;

    // If we have already loaded the items, just exit
    if (m_defaultItemsLoaded)
        return;
    m_defaultItemsLoaded = true;

    // If there are items in the model, no need to load the defaults
    if (count() != 0)
        return;

    // Did we already load the defaults for this agent?
    QStringList alreadyInitialized = m_config.readEntry("defaultItemsProcessedFor", QStringList());
    if (alreadyInitialized.contains(m_shownAgents.first()))
        return;
    alreadyInitialized << m_shownAgents.first();
    m_config.writeEntry("defaultItemsProcessedFor", alreadyInitialized);
    m_config.sync();

    QStringList args = m_defaultItemsConfig.split("/");
    QString configField = args.takeLast();
    QString configGroup = args.takeLast();
    QString configFile = args.join("/");

    // qDebug() << "Config"
    //          << configFile << " "
    //          << configGroup << " "
    //          << configField << " ";

    QStringList items = KSharedConfig::openConfig(configFile)->group(configGroup).readEntry(configField, QStringList());

    for (const auto &item : items) {
        // qDebug() << "Adding: " << item;
        linkResourceToActivity(item, ":global", QJSValue());
    }
}

QString ResourceModel::validateResource(const QString &resource) const
{
    return resource.startsWith(QLatin1String("file://")) ? QUrl(resource).toLocalFile() : resource;
}

} // namespace Imports
} // namespace KActivities

// #include "resourcemodel.moc"
