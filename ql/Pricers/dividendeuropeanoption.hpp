
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

/*! \file dividendeuropeanoption.hpp
    \brief European option with discrete deterministic dividends
*/

#ifndef dividend_european_option_pricer_hpp
#define dividend_european_option_pricer_hpp

#include <ql/option.hpp>

namespace QuantLib {

#ifndef QL_DISABLE_DEPRECATED
    //! European option with dividends
    /*! \deprecated use DividendVanillaOption with 
                    AnalyticDividendEuropeanEngine instead
    */
    class DividendEuropeanOption {
      public:
        DividendEuropeanOption(Option::Type type, Real underlying,
                               Real strike, Spread dividendYield, 
                               Rate riskFreeRate, Time residualTime, 
                               Volatility volatility,
                               const std::vector<Real>& dividends,
                               const std::vector<Time>& exdivdates);
        Real value() const { return value_; }
        Real delta() const { return delta_; }
        Real gamma() const { return gamma_; }
        Real theta() const { return theta_; }
        Real vega() const { return vega_; }
        Real rho() const { return rho_; }
      private:
        Real value_;
        Real delta_, gamma_;
        Real theta_;
        Real vega_;
        Real rho_;
    };

#endif
}


#endif
