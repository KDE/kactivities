/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Self
#include "TemplatesPlugin.h"

// Qt
#include <QStringList>
#include <QString>

// KDE
#include <KDBusConnectionPool>

// Utils
#include <utils/for_each_assoc.h>

// Local
#include "templatesadaptor.h"

KAMD_EXPORT_PLUGIN(templatesplugin, TemplatesPlugin, "kactivitymanagerd-plugin-activitytemplates.json")


TemplatesPlugin::TemplatesPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
{
    Q_UNUSED(args);

    setName(QStringLiteral("org.kde.ActivityManager.ActivityTemplates"));

    new TemplatesAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(QStringLiteral("/Templates"), this);
}

TemplatesPlugin::~TemplatesPlugin()
{
}

bool TemplatesPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_activities = modules[QStringLiteral("activities")];

    return true;
}

QDBusVariant TemplatesPlugin::featureValue(const QStringList &property) const
{
    if (property.size() == 0) {
        return QDBusVariant(QString());
    }

    const auto& id = property[0];

    KConfigGroup pluginConfig(config());
    KConfigGroup activityConfig(&pluginConfig, id);

    if (property.size() == 2) {
        const auto& key = property[1];
        return QDBusVariant(activityConfig.readEntry(key, QVariant(QString())));

    } else {
        return QDBusVariant(activityConfig.keyList());

    }
}

void TemplatesPlugin::setFeatureValue(const QStringList &property,
        const QDBusVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);

    // TODO: Remove. For testing purposes only
    if (property.size() == 1 && property[0] == "createActivity") {
        QVariantHash info {
            { "activity.name", value.variant().toString() },
            { "activity.description", "Nothing special" },
            { "activity.wallpaper", "stripes.png" },
            { "activity.icon", "kate" },
            { "activity.cloneFrom", "id" },
            { "activity.linkedResources", QStringList { "a", "b", "c" } }
        };
        createActivity(QDBusVariant(info));

    }
}

void TemplatesPlugin::createActivity(const QDBusVariant &_values)
{
    using namespace kamd::utils;

    QVariantHash values = _values.variant().toHash();

    auto takeStringValue = [&values] (const QString &key) {
        auto result = values[key].toString();
        values.remove(key);
        return result;
    };

    const QString name        = takeStringValue("activity.name");
    const QString description = takeStringValue("activity.description");
    const QString icon        = takeStringValue("activity.icon");

    // Creating the activity, and getting the id
    const QString id = Plugin::retrieve<QString>(
        m_activities, "AddActivity", "QString",
        Q_ARG(QString, name));

    // Saving the provided data to the configuration file
    KConfigGroup pluginConfig(config());
    KConfigGroup activityConfig(&pluginConfig, id);

    for_each_assoc(values, [&activityConfig] (const QString &key, const QVariant &value) {
        activityConfig.writeEntry(key, value);
    });

    activityConfig.sync();

    // Changing the icon and description of the activity
    Plugin::invoke<Qt::DirectConnection>(
        m_activities, "SetActivityDescription",
        Q_ARG(QString, id),
        Q_ARG(QString, description));
    Plugin::invoke<Qt::DirectConnection>(
        m_activities, "SetActivityIcon",
        Q_ARG(QString, id),
        Q_ARG(QString, icon));

}

#include "TemplatesPlugin.moc"

