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

/*! \file finitedifferenceeuropean.h
    \brief Simple example of european option calculated using finite differences

    $Source$
    $Log$
    Revision 1.1  2001/03/07 17:16:11  marmar
    Example of european option using finite differences

*/

#ifndef quantlib_pricers_finite_difference_european_option_h
#define quantlib_pricers_finite_difference_european_option_h

#include "bsmnumericaloption.h"
#include "standardfdmodel.h"

namespace QuantLib {

    namespace Pricers {

        class FiniteDifferenceEuropean : public BSMNumericalOption{
          public:
            FiniteDifferenceEuropean(Type type, 
                 double underlying, double strike, 
                 Rate dividendYield, Rate riskFreeRate, 
                 Time residualTime, double volatility, 
                 int timeSteps = 200, int gridPoints = 800);            
            double value() const;     
            Handle<BSMOption> clone() const{    
                return Handle<BSMOption>(new FiniteDifferenceEuropean(*this));
            }
          private:
            int timeSteps_;
        };
        
    }

}

#endif
