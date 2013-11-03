/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PLUGINS_GLOBAL_TEMPLATES_H
#define PLUGINS_GLOBAL_TEMPLATES_H

#include <Plugin.h>

class QSignalMapper;
class KActionCollection;

class TemplatesPlugin : public Plugin {
    Q_OBJECT
    // Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.SLC")
    Q_PLUGIN_METADATA(IID "org.kde.ActivityManager.plugins.activitytemplates")

public:
    TemplatesPlugin(QObject *parent = Q_NULLPTR,
                    const QVariantList &args = QVariantList());
    virtual ~TemplatesPlugin();

    virtual bool init(const QHash<QString, QObject *> &modules) Q_DECL_OVERRIDE;

    virtual QDBusVariant value(const QStringList &property) const Q_DECL_OVERRIDE;

    virtual void setValue(const QStringList &property,
                          const QDBusVariant &value) Q_DECL_OVERRIDE;

private:
    inline QStringList activities() const;
    inline QStringList templates() const;
    inline QStringList templateFor(const QString &activity) const;

    QObject *m_activities;

};

#endif // PLUGINS_GLOBAL_TEMPLATES_
