
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

/*! \file dividendoption.hpp
    \brief base class for option with dividends

    $Source$
    $Log$
    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/06 16:11:54  marmar
    Bug fixed in multi-period option

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/03/21 09:56:47  marmar
    virtual method added

    Revision 1.1  2001/03/20 15:15:55  marmar
    DividendOption and DividendShoutOption are examples of
     MultiPeriodOption's

*/

#ifndef shaft_dividend_option_pricer_h
#define shaft_dividend_option_pricer_h

#include "qldefines.hpp"
#include "multiperiodoption.hpp"

namespace QuantLib {

    namespace Pricers {

        class DividendOption : public MultiPeriodOption {
          public:
            // constructor
            DividendOption(Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends = std::vector<double>(),
                const std::vector<Time>& exdivdates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);
            protected:
            void initializeControlVariate() const;
            private:
            std::vector<double> dividends_;

            void executeIntermediateStep(int step) const;

            void movePricesBeforeExDiv(double Div, const Array& newGrid,
                Array& prices, const Array& oldGrid) const;
            double addElements(const std::vector<double>& A) const{
                return std::accumulate(A.begin(), A.end(), 0.0);
            }
        };

    }

}

#endif
