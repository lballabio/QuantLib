/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl

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

/*! \file ql/functional.hpp
    \brief Maps function, bind and cref to either the boost or std implementation
*/

#ifndef quantlib_functional_hpp
#define quantlib_functional_hpp

#include <ql/qldefines.hpp>
#include <functional>

namespace QuantLib {

    namespace ext {

        using std::function;                       // NOLINT(misc-unused-using-decls)
        using std::bind;                           // NOLINT(misc-unused-using-decls)
        using std::ref;                            // NOLINT(misc-unused-using-decls)
        using std::cref;                           // NOLINT(misc-unused-using-decls)
        namespace placeholders {
            using namespace std::placeholders;     // NOLINT(misc-unused-using-decls)
        }
        /*! \deprecated To check if a function is empty, use it in a bool context
                        instead of comparing it to QL_NULL_FUNCTION.
                        Deprecated in version 1.32.
        */
        #define QL_NULL_FUNCTION nullptr

    }

}


#endif

