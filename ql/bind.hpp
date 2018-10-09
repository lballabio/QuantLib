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

/*! \file bind.hpp
    \brief Maps bind to either the boost or std implementation
*/

#ifndef quantlib_bind_hpp
#define quantlib_bind_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_BIND)
#include <functional>
#else
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#endif

namespace QuantLib {

    namespace ext {

        #if defined(QL_USE_STD_BIND)
        using std::bind;
        using std::ref;
        using std::cref;
        namespace placeholders {
            using namespace std::placeholders;
        }
        #else
        using boost::bind;
        using boost::ref;
        using boost::cref;
        namespace placeholders {}
        #endif

    }

}


#endif

