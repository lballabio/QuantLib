
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
    \brief roofed Asian option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.7  2001/07/05 15:57:23  lballabio
// Collected typedefs in a single file
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pagoda_option_pricer_h
#define quantlib_pagoda_option_pricer_h

#include "ql/types.hpp"
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
