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

/*! \file shared_ptr.hpp
    \brief Maps shared_ptr to either the boost or std implementation
*/

#ifndef quantlib_shared_ptr_hpp
#define quantlib_shared_ptr_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_SHARED_PTR)
#include <memory>
#else
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#endif

namespace QuantLib::ext {

        #if defined(QL_USE_STD_SHARED_PTR)
        using std::shared_ptr;                   // NOLINT(misc-unused-using-decls)
        using std::weak_ptr;                     // NOLINT(misc-unused-using-decls)
        using std::make_shared;                  // NOLINT(misc-unused-using-decls)
        using std::static_pointer_cast;          // NOLINT(misc-unused-using-decls)
        using std::dynamic_pointer_cast;         // NOLINT(misc-unused-using-decls)
        using std::enable_shared_from_this;      // NOLINT(misc-unused-using-decls)
        #else
        using boost::shared_ptr;                 // NOLINT(misc-unused-using-decls)
        using boost::weak_ptr;                   // NOLINT(misc-unused-using-decls)
        using boost::make_shared;                // NOLINT(misc-unused-using-decls)
        using boost::static_pointer_cast;        // NOLINT(misc-unused-using-decls)
        using boost::dynamic_pointer_cast;       // NOLINT(misc-unused-using-decls)
        using boost::enable_shared_from_this;    // NOLINT(misc-unused-using-decls)
        #endif

    }


#endif

