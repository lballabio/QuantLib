/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Jonathan Sweemer

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file optional.hpp
    \brief Maps optional to either the boost or std implementation
*/

#ifndef quantlib_optional_hpp
#define quantlib_optional_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_OPTIONAL)
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace QuantLib {

    namespace ext {

        #if defined(QL_USE_STD_OPTIONAL)
        using std::optional;                    // NOLINT(misc-unused-using-decls)
        // here we can assume C++17
        inline constexpr const std::nullopt_t& nullopt = std::nullopt;
        #else
        using boost::optional;                  // NOLINT(misc-unused-using-decls)
        // here we can't
        extern const boost::none_t& nullopt;
        #endif

    }

}

#endif
