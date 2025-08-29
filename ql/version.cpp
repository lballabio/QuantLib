/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Aprexo Limited

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

#include <ql/version.hpp>
#include <boost/version.hpp>

// Macros for forcing the compiler not to inline code
// For now only used here, but could move this to a common header if necessary
#if defined(BOOST_MSVC)       // Microsoft Visual C++
#define QL_FORCE_NONINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define QL_FORCE_NONINLINE __attribute__((noinline))
#else
// we don't know how to enable it, just define the macro away and emit a warning
#define QL_FORCE_NONINLINE
#warning QL_FORCE_NONINLINE is not implemented on this platform
#endif


namespace QuantLib {

    QL_FORCE_NONINLINE std::size_t compiledBoostVersion()
    {
        return static_cast<std::size_t>(BOOST_VERSION);
    }

}
