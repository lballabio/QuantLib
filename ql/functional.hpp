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

/*! \file ql/functional.hpp
    \brief Maps function, bind and cref to either the boost or std implementation
*/

#ifndef quantlib_functional_hpp
#define quantlib_functional_hpp

#include <ql/qldefines.hpp>
#include <functional>

namespace QuantLib::ext {

        /*! \deprecated Use std::function instead.
                        Deprecated in version 1.36.
        */
        template <typename... Ts>
        using function [[deprecated("Use std::function instead")]] = std::function<Ts...>;     // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::bind instead.
                        Deprecated in version 1.36.
        */
        using std::bind;                           // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::ref instead.
                        Deprecated in version 1.36.
        */
        using std::ref;                            // NOLINT(misc-unused-using-decls)

        /*! \deprecated Use std::cref instead.
                        Deprecated in version 1.36.
        */
        using std::cref;                           // NOLINT(misc-unused-using-decls)

#if defined(__GNUC__) && (__GNUC__ <= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

        /*! \deprecated Use the std::placeholders namespace instead.
                        Deprecated in version 1.36.
        */
        namespace [[deprecated("Use the std::placeholders namespace instead")]] placeholders {  // NOLINT(modernize-concat-nested-namespaces)
            using namespace std::placeholders;     // NOLINT(misc-unused-using-decls)
        }

#if defined(__GNUC__) && (__GNUC__ <= 9)
#pragma GCC diagnostic pop
#endif

    }


#endif

