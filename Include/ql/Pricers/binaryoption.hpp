
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

/*!  \file binaryoption.hpp
   \brief European style cash-or-nothing option.

  $Source$

  $Log$
  Revision 1.2  2001/05/23 19:30:27  nando
  smoothing #include xx.hpp

  Revision 1.1  2001/04/09 14:05:48  nando
  all the *.hpp moved below the Include/ql level

  Revision 1.2  2001/04/06 18:46:20  nando
  changed Authors, Contributors, Licence and copyright header

  Revision 1.1  2001/04/04 11:07:23  nando
  Headers policy part 1:
  Headers should have a .hpp (lowercase) filename extension
  All *.h renamed to *.hpp

  Revision 1.1  2001/03/01 12:57:38  enri
  class BinaryOption added, test it with binary_option.py

*/


#ifndef quantlib_pricers_binaryoption_h
#define quantlib_pricers_binaryoption_h

#include "ql/Pricers/bsmoption.hpp"

namespace QuantLib
{
    namespace Pricers
    {
        class BinaryOption: public BSMOption
        {
        public:
            BinaryOption(Type type, double underlying, double strike,
                         Rate dividendYield, Rate riskFreeRate, Time residualTime,
                         double volatility, double cashPayoff = 1 );
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            Handle<BSMOption> clone() const;
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




