
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

/*! \file geometricasianoption.hpp
    \brief geometric Asian option

    \fullpath
    ql/Pricers/%geometricasianoption.hpp
*/

// $Id$

#ifndef quantlib_geometric_asian_option_pricer_h
#define quantlib_geometric_asian_option_pricer_h

#include "ql/Pricers/europeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        //! geometric Asian option
        class GeometricAsianOption : public EuropeanOption    {
           public:
            GeometricAsianOption(Option::Type type,
                                 double underlying,
                                 double strike,
                                 Spread dividendYield,
                                 Rate riskFreeRate,
                                 Time residualTime,
                                 double volatility);
            double vega() const;
            double rho() const;
            Handle<SingleAssetOption> clone() const;
        };


        // inline definitions
        
        inline GeometricAsianOption::GeometricAsianOption(Option::Type type,
            double underlying, double strike, Spread dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility)
        : EuropeanOption(type, underlying, strike,
            (riskFreeRate+dividendYield+volatility*volatility/6.0)/2.0,
            riskFreeRate, residualTime, volatility/QL_SQRT(3)) {}

        inline double GeometricAsianOption::rho() const{
            return EuropeanOption::rho()/2.0;
        }

        inline double GeometricAsianOption::vega() const{
            return EuropeanOption::vega()/QL_SQRT(3.0)
                -EuropeanOption::rho()*volatility_*volatility_/4;
        }

        inline Handle<SingleAssetOption> GeometricAsianOption::clone() const{
            return Handle<SingleAssetOption>(new GeometricAsianOption(*this));
        }

    }

}


#endif
