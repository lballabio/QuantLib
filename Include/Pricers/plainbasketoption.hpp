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
/*! \file plainbasketoption.hpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/02 10:59:51  marmar
    Example of Monte Carlo pricer on mutiple assets

*/

#ifndef quantlib_plain_basket_pricer_h
#define quantlib_plain_basket_pricer_h

#include "qldefines.hpp"
#include "rate.hpp"
#include "MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {
        //! This is a simple example of a multi-factor Monte Carlo pricer

        class PlainBasketOption: public MultiFactorPricer {
        public:
            PlainBasketOption(const Array &underlying, const Array 
            &dividendYield, const Math::Matrix &covariance, 
            Rate riskFreeRate,  double residualTime, 
            int timesteps, long samples, long seed=0);
        };

    }

}

#endif
