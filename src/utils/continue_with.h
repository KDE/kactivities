/*
 * continue_with.h
 * Copyright (C) 2014 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef UTILS_CONTINUE_WITH_H
#define UTILS_CONTINUE_WITH_H

#include <QFuture>
#include <QFutureWatcher>

namespace kamd {
namespace utils {

    namespace detail {

    template <typename _ReturnType>
    inline void pass_value(const QFuture<_ReturnType> &future,
                           QJSValue &handler)
    {
        handler.call({ future.result() });
    }

    template <>
    inline void pass_value(const QFuture<void> &future, QJSValue &handler)
    {
        Q_UNUSED(future)
        handler.call({});
    }

    } // namespace detail


    template <typename _ReturnType>
    inline void continue_with(const QFuture<_ReturnType> &future,
                              QJSValue handler)
    {
        auto watcher = new QFutureWatcher<_ReturnType>();
        QObject::connect(watcher, &QFutureWatcherBase::finished, [=]() mutable {
            detail::pass_value(future, handler);
        });
        watcher->setFuture(future);
    }

} // namespace utils
} // namespace kamd

#endif /* !UTILS_CONTINUE_WITH_H */
