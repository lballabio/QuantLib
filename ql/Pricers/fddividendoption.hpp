
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

/*! \file fddividendoption.hpp
    \brief base class for option with dividends
*/

#ifndef quantlib_dividend_option_pricer_h
#define quantlib_dividend_option_pricer_h

#include <ql/Pricers/fdmultiperiodoption.hpp>

namespace QuantLib {

    class FdDividendOption : public FdMultiPeriodOption {
      public:
        // constructor
        FdDividendOption(
                 Option::Type type, double underlying,
                 double strike, Spread dividendYield, Rate riskFreeRate,
                 Time residualTime, double volatility,
                 const std::vector<double>& dividends = std::vector<double>(),
                 const std::vector<Time>& exdivdates = std::vector<Time>(),
                 int timeSteps = 100, int gridPoints = 100);
        double dividendRho() const {
            QL_FAIL("FdDividendOption::dividendRho not implemented yet");
        }
      protected:
        void initializeControlVariate() const;
      private:
        std::vector<double> dividends_;
        void executeIntermediateStep(Size step) const;
        void movePricesBeforeExDiv(Array& prices,
                                   const Array& newGrid,
                                   const Array& oldGrid) const;
        double addElements(const std::vector<double>& A) const{
            return std::accumulate(A.begin(), A.end(), 0.0);
        }
    };

}


#endif
