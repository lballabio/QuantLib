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
/*! \file everestoption.h
    
    $Source$
    $Log$
    Revision 1.1  2001/03/06 16:59:07  marmar
    First, simplified version, of everest option

*/

#ifndef quantlib_pricers_everest_option_h
#define quantlib_pricers_everest_option_h

#include "qldefines.h"
#include "rate.h"
#include "date.h"
#include "multifactorpricer.h"

namespace QuantLib {

    namespace Pricers {

        //! \brief  The everest-type option pricer.
        /*!
        The payoff of an everest option is simply given by the
        final price initial price ratio of the worst performer
        */
        class EverestOption: public MultiFactorPricer {
        public:
            EverestOption(const Array &underlying, 
                const Array &dividendYield, 
                const Math::Matrix &covariance, 
                Rate riskFreeRate, Time residualTime,
                long samples, long seed = 0);             
        };

    }

}

#endif
