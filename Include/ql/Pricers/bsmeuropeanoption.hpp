
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file bsmeuropeanoption.hpp
    \brief european option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.7  2001/07/06 18:24:17  nando
// slight modifications to avoid VisualStudio warnings
//
// Revision 1.6  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include "ql/Pricers/bsmoption.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Black-Scholes-Merton European option
        class BSMEuropeanOption : public BSMOption {
          public:
              // constructor
              BSMEuropeanOption(Type type, double underlying, double strike,
                        Rate dividendYield, Rate riskFreeRate,
                        Time residualTime, double volatility)
              : BSMOption(type, underlying, strike, dividendYield,
                            riskFreeRate, residualTime, volatility) {}
              // accessors
              double value() const;
              double delta() const;
              double gamma() const;
              double theta() const;
              double vega() const;
              double rho() const;
            Handle<BSMOption> clone() const {
                return Handle<BSMOption>(new BSMEuropeanOption(*this)); }
          private:
            // declared as mutable to preserve
            // the logical constness (does this word exist?) of value()
            mutable DiscountFactor dividendDiscount_, riskFreeDiscount_;
            mutable double standardDeviation_;
            mutable double alpha_, beta_, NID1_;
        };

    }

}


#endif
