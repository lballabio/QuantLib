
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/
/*! \file pagodaoption.hpp
    \brief roofed Asian option

    \fullpath
    ql/Pricers/%pagodaoption.hpp
*/

// $Id$

#ifndef quantlib_pagoda_option_pricer_h
#define quantlib_pagoda_option_pricer_h

#include "ql/MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        //! roofed Asian option
        /*! Given a certain portfolio of assets at the end of the period
            it is returned the minimum of a given roof and a certain fraction 
            of the positive portfolio performance.
            If the performance of the portfolio is below then the payoff 
            is null.
        */
        class PagodaOption : public MultiFactorPricer {
          public:
            PagodaOption(const Array& portfolio,
                         double fraction,
                         double roof,
                         const Array& dividendYield,
                         const Math::Matrix& covariance,
                         Rate riskFreeRate,
                         const std::vector<Time>& times,
                         long samples,
                         bool antithetic,
                         long seed = 0);
        };

    }

}


#endif
