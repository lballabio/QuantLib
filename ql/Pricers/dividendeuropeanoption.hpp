
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

    //! European option with dividends
    /*! \deprecated use DividendVanillaOption with 
                    AnalyticDividendEuropeanEngine instead
    */
    class DividendEuropeanOption {
      public:
        DividendEuropeanOption(Option::Type type, double underlying,
                               double strike, Spread dividendYield, 
                               Rate riskFreeRate, Time residualTime, 
                               double volatility,
                               const std::vector<double>& dividends,
                               const std::vector<Time>& exdivdates);
        double value() const { return value_; }
        double delta() const { return delta_; }
        double gamma() const { return gamma_; }
        double theta() const { return theta_; }
        double vega() const { return vega_; }
        double rho() const { return rho_; }
      private:
        double value_;
        double delta_, gamma_;
        double theta_;
        double vega_;
        double rho_;
    };

}


#endif
