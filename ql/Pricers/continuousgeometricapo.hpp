
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file continuousgeometricapo.hpp
    \brief Continuous Geometric Average Price Option (European exercise)

    \fullpath
    ql/Pricers/%continuousgeometricapo.hpp
*/

// $Id$

#ifndef quantlib_continous_geometric_average_price_option_h
#define quantlib_continous_geometric_average_price_option_h

#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Continuous Geometric Average Price Option (European exercise)
        /*! This class implements a continuous geometric average price
            asian option with european exercise.
            The formula is from "Option Pricing Formulas", E. G. Haug (1997)
            pag 96-97

            \todo add Average Strike version and make it backward starting
        */
        class ContinuousGeometricAPO : public EuropeanOption    {
           public:
            ContinuousGeometricAPO(Option::Type type,
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

        inline ContinuousGeometricAPO::ContinuousGeometricAPO(
            Option::Type type, double underlying, double strike,
            Spread dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility)
        : EuropeanOption(type, underlying, strike,
            (riskFreeRate+dividendYield+volatility*volatility/6.0)/2.0,
            riskFreeRate, residualTime, volatility/QL_SQRT(3)) {}

        inline double ContinuousGeometricAPO::rho() const{
            return EuropeanOption::rho()/2.0;
        }

        inline double ContinuousGeometricAPO::vega() const{
            return EuropeanOption::vega()/QL_SQRT(3.0)
                -EuropeanOption::rho()*volatility_*volatility_/4;
        }

        inline Handle<SingleAssetOption> ContinuousGeometricAPO::clone()
          const {
            return Handle<SingleAssetOption>(
                new ContinuousGeometricAPO(*this));
        }

    }

}


#endif
