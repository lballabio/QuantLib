
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

/*! \file dividendamericanoption.cpp
    \brief american option with discrete deterministic dividends

    $Source$
    $Name$
    $Log$
    Revision 1.26  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.25  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.24  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.23  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.22  2001/03/21 09:50:40  marmar
    DividendAmericanOption is now derived from DividendOption

    Revision 1.21  2001/03/06 15:12:35  marmar
    80 column format enforced

    Revision 1.20  2001/03/02 08:36:45  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.

    Revision 1.19  2001/03/01 14:20:36  marmar
    Private-member syntax changed

    Revision 1.18  2001/03/01 13:53:39  marmar
    Standard step condition and finite-difference model introduced

    Revision 1.17  2001/02/19 12:19:29  marmar
    Added trailing _ to protected and private members

    Revision 1.16  2001/02/15 15:31:40  marmar
    Some beauty added to the files

    Revision 1.15  2001/02/13 11:32:41  marmar
    Efficency improved. Also, dividends do not have to be positive
    to allow for negative cash flows

    Revision 1.14  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.13  2001/01/15 13:38:33  lballabio
    Using new cubic spline

    Revision 1.12  2001/01/08 11:44:18  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.11  2001/01/08 10:28:17  lballabio
    Moved Array to Math namespace

    Revision 1.10  2001/01/08 10:12:02  lballabio
    Fixed humongous bug where prices were not modified by dividends

    Revision 1.9  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.8  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "ql/Pricers/dividendamericanoption.hpp"

namespace QuantLib {

    namespace Pricers {
         DividendAmericanOption::DividendAmericanOption(Type type,
            double underlying, double strike, Rate dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates,
            int timeSteps, int gridPoints)
         :DividendOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility, dividends,
            exdivdates, timeSteps, gridPoints){}

        Handle<BSMOption> DividendAmericanOption::clone() const {
            return Handle<BSMOption>(new DividendAmericanOption(*this));
        }

    }

}
