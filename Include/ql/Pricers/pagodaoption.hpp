
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
/*! \file pagodaoption.hpp

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pagoda_option_pricer_h
#define quantlib_pagoda_option_pricer_h

#include "ql/rate.hpp"
#include "ql/MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {
        /*! \brief A pagoda option is a roofed asian option.
        Given a certain portfolio, of assets at the end of the period
        it is returned the minimum of a given roof and a certain fraction of
        the positive portfolio performance.
        If the performance of the portfolio is below then option is null.
        */

        class PagodaOption: public MultiFactorPricer {
          public:
            PagodaOption(const Array &portfolio,
                         double fraction,
                         double roof,
                         double residualTime,
                         const Math::Matrix &covariance,
                         const Array &dividendYield,
                         Rate riskFreeRate,
                         int timesteps,
                         long samples,
                         long seed = 0);
        };

    }

}

#endif
