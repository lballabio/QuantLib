
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

/*! \file dividendeuropeanoption.hpp
    \brief european option with discrete deterministic dividends

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/12 12:17:21  marmar
    Theta is now consistent with DividendAmerican

    Revision 1.1  2001/04/09 14:05:49  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.11  2001/02/14 13:53:19  marmar
    default  constructor has been moved to the cpp file

    Revision 1.10  2001/02/13 10:43:55  marmar
    Efficency improved. Also, dividends do not have to be positive
    to allow for negative cash flows

    Revision 1.9  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.8  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.7  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.6  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

#include "ql/qldefines.hpp"
#include "ql/dataformatters.hpp"
#include "ql/Pricers/bsmeuropeanoption.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class DividendEuropeanOption : public BSMEuropeanOption    {
        public:
            DividendEuropeanOption(Type type, double underlying, double strike,
                Rate dividendYield, Rate riskFreeRate, Time residualTime,
                double volatility, const std::vector<double>& dividends,
                const std::vector<Time>& exdivdates);
            double theta() const;
            double rho() const;
            Handle<BSMOption> clone() const;

            private:
              std::vector<double> dividends_;
              std::vector<Time> exDivDates_;
              double riskless(Rate r, std::vector<double> divs,
                              std::vector<Time> divDates) const;
        };

        inline double DividendEuropeanOption::riskless(Rate r,
            std::vector<double> divs, std::vector<Time> divDates) const{

            double tmp_riskless = 0.0;
            for(int j = 0; j < divs.size(); j++)
                tmp_riskless += divs[j]*QL_EXP(-r*divDates[j]);
            return tmp_riskless;
        }

    }

}

#endif
