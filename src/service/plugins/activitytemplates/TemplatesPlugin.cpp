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

KAMD_EXPORT_PLUGIN(templatesplugin, TemplatesPlugin, "kactivitymanagerd-plugin-activitytemplates.json")


TemplatesPlugin::TemplatesPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
{
    Q_UNUSED(args)

    setName(QStringLiteral("org.kde.ActivityManager.ActivityTemplates"));
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

QStringList TemplatesPlugin::templates() const
{
    return { QStringLiteral("Test Template 1"),
             QStringLiteral("Template 2"),
             QStringLiteral("Default Template"),
             QStringLiteral("Killer Template"),
             QStringLiteral("Failed Attempt") };
}

QStringList TemplatesPlugin::templateFor(const QString &activity) const
{
    Q_UNUSED(activity);
    return { QStringLiteral("Default Template") };
}

QStringList TemplatesPlugin::activities() const
{
    return Plugin::callOnRet<QStringList, Qt::DirectConnection>(
        m_activities, "ListActivities", "QStringList");
}

QDBusVariant TemplatesPlugin::value(const QStringList &property) const
{
    static const auto emptyResult = QStringList();

    return QDBusVariant(
        // no request
        property.size() == 0 ?
            emptyResult :

        // requesting a global variable
        property[0] == QStringLiteral("templates") ?
            templates() :

        // requesting the template of the specified activity
        activities().contains(property[0]) ?
            templateFor(property[0]) :

        // error
            emptyResult
    );

}

void TemplatesPlugin::setValue(const QStringList &property,
        const QDBusVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);
}

#include "TemplatesPlugin.moc"

