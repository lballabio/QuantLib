/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 CompatibL

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

#ifndef quantlib_ad_hpp
#define quantlib_ad_hpp

namespace QuantLib {}

// Namespace of the library from which TapeDouble is invoked
// Used only if a separate flag is set to avoid operators in global scope
#define cl_ext QuantLib

// Class replacing the native double (TapeDouble) and its dependencies
#include <cl/tape/tape.hpp>

namespace QuantLib
{
    template <typename > class Null;

    // Specialization of Null template to make it work with non-native double
    template <>
    class Null<cl::TapeDouble>
    {
    public:
        Null() {}
        cl::TapeDouble operator -()
        {
            return -std::numeric_limits<double>::max();
        }

        inline operator cl::TapeDouble()
        {
            return cl::TapeDouble(std::numeric_limits<double>::max());
        }
    };
}

#endif
