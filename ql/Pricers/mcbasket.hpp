
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file mcbasket.hpp
    \brief simple example of multi-factor Monte Carlo pricer
*/

#ifndef quantlib_basket_pricer_h
#define quantlib_basket_pricer_h

#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>

namespace QuantLib {

#ifndef QL_DISABLE_DEPRECATED
    //! simple example of multi-factor Monte Carlo pricer
    /*! \deprecated use BasketOption instead */
    class McBasket : public McPricer<MultiAsset<PseudoRandom> > {
      public:
        McBasket(Option::Type type,
                 const std::vector<double>& underlyings,
                 double strike,
                 const std::vector<RelinkableHandle<TermStructure> >& 
                                                             dividendYields,
                 const RelinkableHandle<TermStructure>& riskFreeRate,
                 const std::vector<RelinkableHandle<BlackVolTermStructure> >& 
                                                             volatilities,
                 const Matrix& correlation,
                 double residualTime,
                 long seed = 0);
    };
#endif

}


#endif
