
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

/*! \file fddividendamericanoption.hpp
    \brief american option with discrete deterministic dividends
*/

#ifndef quantlib_dividend_american_option_pricer_h
#define quantlib_dividend_american_option_pricer_h

#include <ql/Pricers/fddividendoption.hpp>

namespace QuantLib {

    /*! \bug sometimes yields negative vega when deeply in-the-money
        \bug method impliedVolatility() utterly fails
    */
    class FdDividendAmericanOption : public FdDividendOption {
      public:
        FdDividendAmericanOption(
                 Option::Type type, double underlying,
                 double strike, Spread dividendYield, 
                 Rate riskFreeRate, Time residualTime, 
                 double volatility,
                 const std::vector<double>& dividends = std::vector<double>(),
                 const std::vector<Time>& exdivdates = std::vector<Time>(),
                 int timeSteps = 100, int gridPoints = 100);
        Handle<SingleAssetOption> clone() const;
    };

}


#endif
