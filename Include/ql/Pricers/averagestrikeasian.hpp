
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

/*! \file averagestrikeasian.hpp
    \brief example of Monte Carlo pricer using a control variate

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.6  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pricers_average_strike_asian_pricer_h
#define quantlib_pricers_average_strike_asian_pricer_h

#include "ql/options.hpp"
#include "ql/types.hpp"
#include "ql/MonteCarlo/mcpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        //! example of Monte Carlo pricer using a control variate.
        class AverageStrikeAsian : public McPricer {
          public:
            AverageStrikeAsian(Option::Type type, double underlying,
               double strike, Rate dividendYield, Rate riskFreeRate,
               double residualTime, double volatility, int timesteps,
               long samples, long seed = 0);
        };

    }

}


#endif
