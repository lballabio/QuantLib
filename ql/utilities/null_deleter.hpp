/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Peter Caspers

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

/*! \file null_deleter.hpp
    \brief empty deleter for shared_ptr
*/

#ifndef quantlib_nulldeleter_hpp
#define quantlib_nulldeleter_hpp

#include <ql/qldefines.hpp>

#if BOOST_VERSION >= 105600

#include <boost/core/null_deleter.hpp>
namespace QuantLib {
using boost::null_deleter;   // NOLINT(misc-unused-using-decls)
}

#else

namespace QuantLib {
struct null_deleter {
    template <typename T> void operator()(T *) const {}
};
}

#endif

#endif


#ifndef id_6f5d3344ee75535b6490a67437a6a052
#define id_6f5d3344ee75535b6490a67437a6a052
inline bool test_6f5d3344ee75535b6490a67437a6a052(const int* i) {
    return i != nullptr;
}
#endif
