
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
                          double underlying,
                          double moneyness,
                          const std::vector<Spread>& dividendYield,
                          const std::vector<Rate>& riskFreeRate,
                          const std::vector<Time>& times,
                          const std::vector<double>& volatility);
        double value() const { return value_; }
        double delta() const { return delta_; }
        double gamma() const { return gamma_; }
        double theta() const { return theta_; }
        double vega() const { return vega_; }
        double rho() const { return rho_; }
        double dividendRho() const { return dividendRho_; }
      private:
        double value_;
        double delta_, gamma_;
        double theta_;
        double vega_;
        double rho_, dividendRho_;
    };

#endif

}


#endif

