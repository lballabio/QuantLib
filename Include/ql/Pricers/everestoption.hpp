
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
/*! \file everestoption.hpp

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pricers_everest_option_h
#define quantlib_pricers_everest_option_h

#include "ql/rate.hpp"
#include "ql/date.hpp"
#include "ql/MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        //! \brief  The everest-type option pricer.
        /*!
        The payoff of an everest option is simply given by the
        final price initial price ratio of the worst performer
        */
        class EverestOption: public MultiFactorPricer {
        public:
            EverestOption(const Array &dividendYield,
                const Math::Matrix &covariance,
                Rate riskFreeRate, Time residualTime,
                long samples, long seed = 0);
        };

    }

}

#endif
