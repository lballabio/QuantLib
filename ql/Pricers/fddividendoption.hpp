
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
                 Option::Type type, Real underlying,
                 Real strike, Spread dividendYield, Rate riskFreeRate,
                 Time residualTime, Volatility volatility,
                 const std::vector<Real>& dividends = std::vector<Real>(),
                 const std::vector<Time>& exdivdates = std::vector<Time>(),
                 Size timeSteps = 100, Size gridPoints = 100);
        Real dividendRho() const {
            QL_FAIL("not implemented");
        }
      protected:
        void initializeControlVariate() const;
      private:
        std::vector<Real> dividends_;
        void executeIntermediateStep(Size step) const;
        void movePricesBeforeExDiv(Array& prices,
                                   const Array& newGrid,
                                   const Array& oldGrid) const;
        Real addElements(const std::vector<Real>& A) const{
            return std::accumulate(A.begin(), A.end(), 0.0);
        }
    };

}


#endif
