
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
    $Log$
    Revision 1.4  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

    Revision 1.3  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.2  2001/04/12 12:17:21  marmar
    Theta is now consistent with DividendAmerican

*/

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

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
