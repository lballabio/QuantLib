

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*!  \file binaryoption.hpp
    \brief European style cash-or-nothing option.

    \fullpath
    ql/Pricers/%binaryoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_binaryoption_h
#define quantlib_pricers_binaryoption_h

#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib  {

    namespace Pricers {

        //! Binary (digital) option
        class BinaryOption : public SingleAssetOption {
          public:
            BinaryOption(Option::Type type, double underlying, double strike,
                         Spread dividendYield, Rate riskFreeRate,
                         Time residualTime, double volatility,
                         double cashPayoff = 1 );
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            Handle<SingleAssetOption> clone() const;
        private:
            double cashPayoff_;
            double beta_;
            double discount_;
            double volSqrtTime_;
            double optionSign_;
            double D2_;
            double NID2_;
            double ND2_;
            double D1_;
        };
    }
}


#endif




