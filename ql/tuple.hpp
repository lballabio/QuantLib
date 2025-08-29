/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file tuple.hpp
    \brief Maps tuple to either the boost or std implementation
*/

#ifndef quantlib_tuple_hpp
#define quantlib_tuple_hpp

#include <ql/qldefines.hpp>
#include <tuple>

namespace QuantLib::ext {

        /*! \deprecated Use std::tuple instead.
                        Deprecated in version 1.36.
        */
        template <typename... Ts>
        using tuple [[deprecated("Use std::tuple instead")]] = std::tuple<Ts...>;    // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::make_tuple instead.
                        Deprecated in version 1.36.
        */
        using std::make_tuple;              // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::get instead.
                        Deprecated in version 1.36.
        */
        using std::get;                     // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::tie or structured bindings instead.
                        Deprecated in version 1.36.
        */
        using std::tie;                     // NOLINT(misc-unused-using-decls)

    }


#endif

