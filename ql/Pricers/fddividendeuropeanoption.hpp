

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
