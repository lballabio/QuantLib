
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
/*! \file dividendshoutoption.hpp
    \brief base class for shout option with dividends

    \fullpath
    ql/Pricers/%dividendshoutoption.hpp
*/

// $Id$

#ifndef quantlib_dividend_shout_option_pricer_h
#define quantlib_dividend_shout_option_pricer_h

#include <ql/Pricers/fddividendoption.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Shout option with dividends
        class FdDividendShoutOption : public FdDividendOption {
          public:
            // constructor
            FdDividendShoutOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends = std::vector<double>(),
                const std::vector<Time>& exdivdates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);

            Handle<SingleAssetOption> clone() const;
            double dividendRho() const {
                throw Error("FdDividendOption::dividendRho not implemented yet");
            }
          protected:
            void initializeStepCondition() const;
        };

    }

}


#endif
