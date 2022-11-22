/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file pathpricer.hpp
    \brief base class for single-path pricers
*/

#ifndef quantlib_montecarlo_path_pricer_hpp
#define quantlib_montecarlo_path_pricer_hpp

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! base class for path pricers
    /*! Returns the value of an option on a given path.

        \ingroup mcarlo
    */
    template<class PathType, class ValueType=Real>
    class PathPricer {
      public:
        typedef PathType argument_type;
        typedef ValueType result_type;

        virtual ~PathPricer() = default;
        virtual ValueType operator()(const PathType& path) const=0;
    };

}


#endif
