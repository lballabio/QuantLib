
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

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.7  2001/02/20 11:14:37  marmar
    "growth" replaced with dividend

    Revision 1.6  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.5  2001/01/16 11:23:27  nando
    removed tabs and enforced 80 columns

    Revision 1.4  2000/12/14 12:40:13  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include "ql/qldefines.hpp"
#include "ql/Pricers/bsmoption.hpp"
#include "ql/discountfactor.hpp"

namespace QuantLib {

    namespace Pricers {

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
