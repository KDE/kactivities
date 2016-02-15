/*
 *   Copyright (C) 2012 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef D_PTR_H
#define D_PTR_H

#include <memory>

namespace kamd {
namespace utils {

template <typename T>
class d_ptr {
private:
    std::unique_ptr<T> d;

public:
    d_ptr();

    template <typename... Args>
    d_ptr(Args &&...);

    ~d_ptr();

    T *operator->() const;
};

#define D_PTR             \
    class Private;        \
    friend class Private; \
    const ::kamd::utils::d_ptr<Private> d

} // namespace utils
} // namespace kamd

#endif
