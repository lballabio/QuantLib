
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file payoff.hpp
    \brief Option payoff classes
*/

#ifndef quantlib_payoff_hpp
#define quantlib_payoff_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Base class for option payoffs
    class Payoff : std::unary_function<Real,Real> {
      public:
        virtual ~Payoff() {}
        virtual Real operator()(Real price) const = 0;
    };

}


#endif
