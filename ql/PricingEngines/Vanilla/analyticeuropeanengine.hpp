
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003, 2004 StatPro Italia srl

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

/*! \file analyticeuropeanengine.hpp
    \brief Analytic European engine
*/

#ifndef quantlib_analytic_european_engine_hpp
#define quantlib_analytic_european_engine_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! Pricing engine for European vanilla options using analytical formulae
    /*! \ingroup vanillaengines

        \test a) the correctness of the returned value is tested by
                 reproducing results available in literature.

        \test b) the correctness of the returned greeks is tested by
                 reproducing results available in literature.

        \test c) the correctness of the returned greeks is tested by
                 reproducing numerical derivatives.

        \test d) the correctness of the returned implied volatility is
                 tested by using it for reproducing the target value.

        \test e) the implied-volatility calculation is tested by
                 checking that it does not modify the option.

        \test f) the correctness of the returned value in case of
                 cash-or-nothing digital payoff is tested by
                 reproducing results available in literature.

        \test g) the correctness of the returned value in case of
                 asset-or-nothing digital payoff is tested by
                 reproducing results available in literature.

        \test h) the correctness of the returned value in case of gap
                 digital payoff is tested by reproducing results
                 available in literature.

        \test i) the correctness of the returned greeks in case of
                 cash-or-nothing digital payoff is tested by
                 reproducing numerical derivatives.
    */
    class AnalyticEuropeanEngine : public VanillaOption::engine {
      public:
        void calculate() const;
    };

}


#endif
