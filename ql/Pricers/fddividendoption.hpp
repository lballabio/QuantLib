
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file dividendoption.hpp
    \brief base class for option with dividends

    \fullpath
    ql/Pricers/%dividendoption.hpp
*/

// $Id$

#ifndef quantlib_dividend_option_pricer_h
#define quantlib_dividend_option_pricer_h

#include <ql/Pricers/fdmultiperiodoption.hpp>

namespace QuantLib {

    namespace Pricers {

        class FdDividendOption : public FdMultiPeriodOption {
          public:
            // constructor
            FdDividendOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends = std::vector<double>(),
                const std::vector<Time>& exdivdates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);
            double dividendRho() const {
                throw Error("FdDividendOption::dividendRho not implemented yet");
            }
            protected:
            void initializeControlVariate() const;
            private:
            std::vector<double> dividends_;

            void executeIntermediateStep(int step) const;

            void movePricesBeforeExDiv(Array& prices,
                                       const Array& newGrid,
                                       const Array& oldGrid) const;
            double addElements(const std::vector<double>& A) const{
                return std::accumulate(A.begin(), A.end(), 0.0);
            }
        };

    }

}


#endif
