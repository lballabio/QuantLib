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

/*! \file tuple.hpp
    \brief Maps tuple to either the boost or std implementation
*/

#ifndef quantlib_tuple_hpp
#define quantlib_tuple_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_TUPLE)
#include <tuple>
#else
#include <boost/tuple/tuple.hpp>
#endif

namespace QuantLib {

    namespace ext {

        #if defined(QL_USE_STD_TUPLE)
        using std::tuple;                   // NOLINT(misc-unused-using-decls)
        using std::make_tuple;              // NOLINT(misc-unused-using-decls)
        using std::get;                     // NOLINT(misc-unused-using-decls)
        using std::tie;                     // NOLINT(misc-unused-using-decls)
        #else
        using boost::tuple;                 // NOLINT(misc-unused-using-decls)
        using boost::make_tuple;            // NOLINT(misc-unused-using-decls)
        using boost::get;                   // NOLINT(misc-unused-using-decls)
        using boost::tie;                   // NOLINT(misc-unused-using-decls)
        #endif

    }

}


#endif

