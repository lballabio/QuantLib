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

/*! \file any.hpp
    \brief Maps any to either the boost or std implementation
*/

#ifndef quantlib_any_hpp
#define quantlib_any_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_ANY)
#include <any>
#else
// Deprecated in version 1.39
#pragma message("Warning: using boost::any is deprecated.  Enable std::any instead.")
#include <boost/any.hpp>
#endif

namespace QuantLib::ext {

        #if defined(QL_USE_STD_ANY)
        using std::any;                   // NOLINT(misc-unused-using-decls)
        using std::any_cast;              // NOLINT(misc-unused-using-decls)
        #else
        using boost::any;                 // NOLINT(misc-unused-using-decls)
        using boost::any_cast;            // NOLINT(misc-unused-using-decls)
        #endif

    }

#endif
