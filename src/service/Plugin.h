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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <kdemacros.h>

#include <QObject>
#include <QMetaObject>

#include <KPluginFactory>
#include <KConfigGroup>

#include "Event.h"
#include "Module.h"

#include <utils/d_ptr.h>

#define KAMD_EXPORT_PLUGIN(ClassName, AboutData)                       \
    K_PLUGIN_FACTORY(ClassName##Factory, registerPlugin<ClassName>();) \
    K_EXPORT_PLUGIN(ClassName##Factory(AboutData))


/**
 *
 */
class KDE_EXPORT Plugin: public Module {
    Q_OBJECT

public:
    Plugin(QObject * parent);
    virtual ~Plugin();

    /**
     * Initializes the plugin.
     * @arg modules Activities, Resources and Features manager objects
     * @returns the plugin needs to return whether it has
     *      successfully been initialized
     */
    virtual bool init(const QHash < QString, QObject * > & modules);

    /**
     * Returns the config group for the plugin.
     * In order to use it, you need to set the plugin name.
     */
    KConfigGroup config();
    QString name() const;

    /**
     * Convenience meta-method to provide prettier invocation of QMetaObject::invokeMethod
     */
    template <typename ReturnType, Qt::ConnectionType connection>
    static ReturnType callOn(QObject * object, const char * method, const char * returnTypeName)
    {
        ReturnType result;

        QMetaObject::invokeMethod(
                object, method, connection,
                QReturnArgument < ReturnType > (returnTypeName, result)
            );

        return result;
    }

    template <typename ReturnType, Qt::ConnectionType connection, typename... Args>
    static ReturnType callOnWithArgs(QObject * object, const char * method, const char * returnTypeName, Args ... args)
    {
        ReturnType result;

        QMetaObject::invokeMethod(
                object, method, connection,
                QReturnArgument < ReturnType > (returnTypeName, result),
                args...
            );

        return result;
    }

protected:
    void setName(const QString & name);

private:
    D_PTR;

};

#endif // PLUGIN_H

