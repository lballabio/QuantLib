/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

/*! \file marshallolkincopula.hpp
    \brief Marshall-Olkin copula
*/

#ifndef quantlib_math_MarshallOlkin_copula_h
#define quantlib_math_MarshallOlkin_copula_h

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Marshall-Olkin copula
    class MarshallOlkinCopula {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;

        MarshallOlkinCopula(Real a1, Real a2);
        Real operator()(Real x, Real y) const;
      private:
        Real a1_, a2_;
    };
    
}

#endif
