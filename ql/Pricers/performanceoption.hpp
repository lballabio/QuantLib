
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file performanceoption.hpp
    \brief Performance option
*/

#ifndef quantlib_pricers_performance_option_hpp
#define quantlib_pricers_performance_option_hpp

#include <ql/option.hpp>

namespace QuantLib {

#ifndef QL_DISABLE_DEPRECATED

    //! Performance option
    /*! A performance option is a variant of a cliquet option: the
        payoff of each forward-starting (a.k.a. deferred strike)
        options is \$ max(S/X- 1) \$.

        \deprecated Use CliquetOption with AnalyticPerformanceEngine instead
    */
    class PerformanceOption {
      public:
        PerformanceOption(Option::Type type,
                          Real underlying,
                          Real moneyness,
                          const std::vector<Spread>& dividendYield,
                          const std::vector<Rate>& riskFreeRate,
                          const std::vector<Time>& times,
                          const std::vector<Volatility>& volatility);
        Real value() const { return value_; }
        Real delta() const { return delta_; }
        Real gamma() const { return gamma_; }
        Real theta() const { return theta_; }
        Real vega() const { return vega_; }
        Real rho() const { return rho_; }
        Real dividendRho() const { return dividendRho_; }
      private:
        Real value_;
        Real delta_, gamma_;
        Real theta_;
        Real vega_;
        Real rho_, dividendRho_;
    };

#endif

}


#endif

