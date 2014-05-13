/*
 * continue_with.h
 * Copyright (C) 2014 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef UTILS_CONTINUE_WITH_H
#define UTILS_CONTINUE_WITH_H

namespace kamd {
namespace utils {

    template <typename _ReturnType>
    void continue_with(const QFuture<_ReturnType> &future, QJSValue handler)
    {
        auto watcher = new QFutureWatcher<_ReturnType>();
        QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] () mutable {
                    handler.call(QJSValueList() << future.result());
                }
            );
        watcher->setFuture(future);
    }

    template <>
    void continue_with(const QFuture<void> &future, QJSValue handler)
    {
        auto watcher = new QFutureWatcher<void>();
        QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] () mutable {
                    handler.call(QJSValueList());
                }
            );
        watcher->setFuture(future);
    }

} // namespace utils
} // namespace kamd

#endif /* !UTILS_CONTINUE_WITH_H */
