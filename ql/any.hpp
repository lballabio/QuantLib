/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Jonathan Sweemer

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

/*! \file any.hpp
    \brief Maps std::any to the std implementation
*/

#ifndef quantlib_any_hpp
#define quantlib_any_hpp

#include <ql/qldefines.hpp>
#include <any>

namespace QuantLib::ext {

    /*! \deprecated Use std::any instead.
                    Deprecated in version 1.44.
    */
    using any [[deprecated("Use std::any instead")]] = std::any; // NOLINT(misc-unused-using-decls)

    /*! \deprecated Use std::any_cast instead.
                    Deprecated in version 1.44.
    */
    using std::any_cast;  // NOLINT(misc-unused-using-decls)

}

#endif
