
     
/*
 * Copyright (C) 2000, 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at 
 *      http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file geometricasianoption.hpp

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/02/19 12:20:21  marmar
    Added trailing _ to protected and private members

    Revision 1.3  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.2  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef    quantlib_geometric_asian_option_pricer_h
#define    quantlib_geometric_asian_option_pricer_h

#include "qldefines.hpp"
#include "bsmeuropeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        class GeometricAsianOption : public BSMEuropeanOption    {
           public:
            GeometricAsianOption(Type type, double underlying, double    strike,
                Rate dividendYield, Rate riskFreeRate, Time    residualTime,
                double volatility);
            double vega() const;
            double rho() const;
            Handle<BSMOption> clone() const;
        };

        inline GeometricAsianOption::GeometricAsianOption(Type type, 
            double underlying, double strike, Rate dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility):
            BSMEuropeanOption(type, underlying, strike, dividendYield/2,
            riskFreeRate/2-volatility*volatility/12, residualTime, 
            volatility/QL_SQRT(3)){}

        inline double GeometricAsianOption::rho() const{
            return BSMEuropeanOption::rho()/2;
        }

        inline double GeometricAsianOption::vega() const{
            return BSMEuropeanOption::vega()/QL_SQRT(3)
                -BSMEuropeanOption::rho()*volatility_*volatility_/4;
        }

        inline Handle<BSMOption> GeometricAsianOption::clone() const{
            return Handle<BSMOption>(new GeometricAsianOption(*this));
        }

    }

}

#endif
