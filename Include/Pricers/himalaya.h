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
/*! \file himalaya.h
    
    $Source$
    $Log$
    Revision 1.2  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.1  2001/02/07 10:14:43  marmar
    Himalaya-type option pricer

*/

#ifndef quantlib_himalaya_h
#define quantlib_himalaya_h

#include "qldefines.h"
#include "rate.h"
#include "date.h"
#include "multifactorpricer.h"

namespace QuantLib {

    namespace Pricers {

        //! \brief  The himalayan-type option pricer.
        /*!
        The payoff of an himalaya option is computed in the following way:
            Given a basket of N assets, and N time periods, at end of 
            each period the option who performed the best is added to the 
            average and then discarded from the basket. At the end of the 
            N periods the option pays the max between the strike and the 
            average of the best performers.
        */
        class Himalaya: public MultiFactorPricer {
        public:
            Himalaya(const Array &underlying, 
                const Array &dividendYield, 
                const Math::Matrix &covariance, 
                Rate riskFreeRate, double strike, 
                const std::vector<Time> &timeDelays, 
                long samples, long seed = 0);
        };

    }

}

#endif
