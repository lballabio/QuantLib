
/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmamericanoption.h
    \brief american option

    $Source$
    $Name$
    $Log$
    Revision 1.7  2001/03/01 13:57:36  marmar
    theTimeSteps changed in timeSteps_

    Revision 1.6  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.5  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.4  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_american_option_pricer_h
#define BSM_american_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "americancondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {

    namespace Pricers {

        class BSMAmericanOption : public BSMNumericalOption {
          public:
            // constructor
            BSMAmericanOption(Type type, double underlying, double strike, 
                Rate dividendYield, Rate riskFreeRate, Time residualTime, 
                double volatility, int timeSteps, int gridPoints)
            : BSMNumericalOption(type, underlying, strike, dividendYield, 
                riskFreeRate, residualTime, volatility, gridPoints), 
                timeSteps_(timeSteps) {}
            // accessors
            double value() const;
            // This method must be implemented to imply volatilities
            Handle<BSMOption> clone() const{    
                return Handle<BSMOption>(new BSMAmericanOption(*this));
            }
          private:
            int timeSteps_;
        };

    }

}


#endif
