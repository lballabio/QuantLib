
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

/*! \file dividendamericanoption.hpp
    \fullpath Include/ql/Pricers/%dividendamericanoption.hpp
    \brief american option with discrete deterministic dividends

*/

// $Id$
// $Log$
// Revision 1.14  2001/08/21 10:47:26  nando
// "dividendRho not implemented yet" moved up in the class hierarchy
//
// Revision 1.13  2001/08/13 15:06:17  nando
// added dividendRho method
//
// Revision 1.12  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.11  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.6  2001/06/05 12:45:27  nando
// R019-branch-merge4 merged into trunk
//


#ifndef quantlib_dividend_american_option_pricer_h
#define quantlib_dividend_american_option_pricer_h

#include "ql/Pricers/dividendoption.hpp"

namespace QuantLib {

    namespace Pricers {

        class DividendAmericanOption : public DividendOption {
          public:
            // constructor
            DividendAmericanOption(Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends = std::vector<double>(),
                const std::vector<Time>& exdivdates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);
            Handle<SingleAssetOption> clone() const;
        };

    }

}


#endif
