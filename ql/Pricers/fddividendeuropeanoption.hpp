
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

/*! \file dividendeuropeanoption.hpp
    \brief european option with discrete deterministic dividends

    \fullpath
    ql/Pricers/%dividendeuropeanoption.hpp
*/

// $Id$

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

#include <ql/Pricers/europeanoption.hpp>
#include <vector>

namespace QuantLib {

    namespace Pricers {

        //! European option with dividends
        class FdDividendEuropeanOption : public EuropeanOption    {
          public:
            FdDividendEuropeanOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends,
                const std::vector<Time>& exdivdates);
            double theta() const;
            double rho() const;
            double dividendRho() const {
                throw Error("FdDividendEuropeanOption::dividendRho not"
                    "implemented yet");
            }
            Handle<SingleAssetOption> clone() const;
          private:
            std::vector<double> dividends_;
            std::vector<Time> exDivDates_;
            double riskless(Rate r, std::vector<double> divs,
                            std::vector<Time> divDates) const;
        };


        // inline definitions

        inline double FdDividendEuropeanOption::riskless(Rate r,
            std::vector<double> divs, std::vector<Time> divDates) const{

            double tmp_riskless = 0.0;
            for(Size j = 0; j < divs.size(); j++)
                tmp_riskless += divs[j]*QL_EXP(-r*divDates[j]);
            return tmp_riskless;
        }

    }

}


#endif
