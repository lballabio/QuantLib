
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

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef shaft_dividend_option_pricer_h
#define shaft_dividend_option_pricer_h

#include "ql/Pricers/multiperiodoption.hpp"

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

            void movePricesBeforeExDiv(Array& prices,
                                       const Array& newGrid,
                                       const Array& oldGrid) const;
            double addElements(const std::vector<double>& A) const{
                return std::accumulate(A.begin(), A.end(), 0.0);
            }
        };

    }

}

#endif
