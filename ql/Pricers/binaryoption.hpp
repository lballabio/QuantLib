
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*!  \file binaryoption.hpp
    \brief European style cash-or-nothing option.

    \fullpath
    ql/Pricers/%binaryoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_binaryoption_h
#define quantlib_pricers_binaryoption_h

#include "ql/Pricers/singleassetoption.hpp"

namespace QuantLib  {
    
    namespace Pricers {

        //! Binary (digital) option        
        class BinaryOption : public SingleAssetOption {
          public:
            BinaryOption(Option::Type type, double underlying, double strike,
                         Rate dividendYield, Rate riskFreeRate, 
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




