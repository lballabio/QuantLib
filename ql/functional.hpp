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

#if defined(QL_USE_STD_FUNCTION)
#include <functional>
#else
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>
#endif

namespace QuantLib {

    namespace ext {

        #if defined(QL_USE_STD_FUNCTION)
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
        #else
        using boost::function;                     // NOLINT(misc-unused-using-decls)
        using boost::bind;                         // NOLINT(misc-unused-using-decls)
        using boost::ref;                          // NOLINT(misc-unused-using-decls)
        using boost::cref;                         // NOLINT(misc-unused-using-decls)
        namespace placeholders {
            #if BOOST_VERSION >= 106000
            using namespace boost::placeholders;   // NOLINT(misc-unused-using-decls)
            #else
            using ::_1;                            // NOLINT(misc-unused-using-decls)
            using ::_2;                            // NOLINT(misc-unused-using-decls)
            using ::_3;                            // NOLINT(misc-unused-using-decls)
            using ::_4;                            // NOLINT(misc-unused-using-decls)
            using ::_5;                            // NOLINT(misc-unused-using-decls)
            using ::_6;                            // NOLINT(misc-unused-using-decls)
            using ::_7;                            // NOLINT(misc-unused-using-decls)
            using ::_8;                            // NOLINT(misc-unused-using-decls)
            using ::_9;                            // NOLINT(misc-unused-using-decls)
            #endif
        }
        /*! \deprecated To check if a function is empty, use it in a bool context
                        instead of comparing it to QL_NULL_FUNCTION.
                        Deprecated in version 1.32.
        */
        #define QL_NULL_FUNCTION 0
        #endif

    }

}


#endif

