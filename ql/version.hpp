/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl
 Copyright (C) 2019 Aprexo Limited

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

/*! \file version.hpp
    \brief Version number, and version of boost the library is compiled with
*/

#ifndef quantlib_version_hpp
#define quantlib_version_hpp

#include <ql/qldefines.hpp>

/*! \addtogroup macros */
/*! @{ */

//! version string
#define QL_VERSION "1.30-dev"

//! version hexadecimal number
#define QL_HEX_VERSION 0x01300000

/*! @}  */

namespace QuantLib {

    /*! Returns the version of boost that the QuantLib library was built with
        Use to check that client code is using a consistent version of boost.
        Using QuantLib header files compiled with a different version of boost
        than the library itself may result in undefined behaviour */
    std::size_t compiledBoostVersion();

}

#endif
