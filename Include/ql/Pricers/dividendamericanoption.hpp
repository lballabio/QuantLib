
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

/*! \file dividendamericanoption.hpp
    \brief american option with discrete deterministic dividends

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:49  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.12  2001/03/21 09:51:01  marmar
    DividendAmericanOption is now derived from DividendOption

    Revision 1.11  2001/02/13 11:33:13  marmar
    Efficency improved. Also, dividends do not have to be positive
    to allow for negative cash flows

    Revision 1.10  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.9  2001/01/10 16:35:35  nando
    timeStepPerDiv was double now it is int

    Revision 1.8  2001/01/08 11:44:17  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.7  2001/01/08 10:28:16  lballabio
    Moved Array to Math namespace

    Revision 1.6  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef shaft_dividend_american_option_pricer_h
#define shaft_dividend_american_option_pricer_h

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
            Handle<BSMOption> clone() const;
        };
    }

}


#endif
