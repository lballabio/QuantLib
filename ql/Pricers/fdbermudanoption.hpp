
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
/*! \file bermudanoption.hpp
    \brief Finite-difference evaluation of Bermudan option

    \fullpath
    ql/Pricers/%bermudanoption.hpp
*/

// $Id$

#ifndef quantlib_bermudan_option_pricer_h
#define quantlib_bermudan_option_pricer_h

#include <ql/Pricers/fdmultiperiodoption.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Bermudan option
        class FdBermudanOption : public FdMultiPeriodOption {
          public:
            // constructor
            FdBermudanOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<Time>& dates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);
            Handle<SingleAssetOption> clone() const;
          protected:
            double extraTermInBermudan ;
            void initializeStepCondition() const;
            void executeIntermediateStep(int ) const;
        };

    }

}


#endif
