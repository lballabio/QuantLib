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
/*! \file plainbasketoption.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/02/02 10:59:51  marmar
    Example of Monte Carlo pricer on mutiple assets

*/

#ifndef quantlib_plain_basket_pricer_h
#define quantlib_plain_basket_pricer_h

#include "qldefines.h"
#include "rate.h"
#include "multifactorpricer.h"

namespace QuantLib {

    namespace Pricers {
        //! This is a simple example of a multi-factor Monte Carlo pricer

        class PlainBasketOption: public MultiFactorPricer {
        public:
            PlainBasketOption(const Array &underlying, const Array 
            &underlyingGrowthRate, const Math::Matrix &covariance, 
            Rate riskFreeRate,  double residualTime, 
            int timesteps, long samples, long seed=0);
        };

    }

}

#endif
