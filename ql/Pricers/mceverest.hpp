
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

/*! \file mceverest.hpp
    \brief %Everest-type option pricer
*/

#ifndef quantlib_pricers_everest_pricer_h
#define quantlib_pricers_everest_pricer_h

#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>

namespace QuantLib {

    //! Everest-type option pricer
    /*! The payoff of an Everest option is simply given by the
        final price / initial price ratio of the worst performer
    */
    class McEverest : public McPricer<MultiAsset<PseudoRandom> > {
      public:
        McEverest(const std::vector<RelinkableHandle<TermStructure> >& 
                                                             dividendYield,
                  const RelinkableHandle<TermStructure>& riskFreeRate,
                  const std::vector<RelinkableHandle<BlackVolTermStructure> >& 
                                                             volatilities,
                  const Matrix& correlation,
                  Time residualTime,
                  BigInteger seed = 0);
    };

}


#endif
