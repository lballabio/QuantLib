
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*!  \file binaryoption.h
   \brief European style cash-or-nothing option.
  
  $Source$

  $Log$
  Revision 1.1  2001/03/01 12:57:38  enri
  class BinaryOption added, test it with binary_option.py

*/


#ifndef quantlib_pricers_binaryoption_h
#define quantlib_pricers_binaryoption_h

#include "qldefines.h"
#include "bsmoption.h"

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




