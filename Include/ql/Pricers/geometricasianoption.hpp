
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
    \fullpath Include/ql/Pricers/%geometricasianoption.hpp
    \brief geometric Asian option

*/

// $Id$
// $Log$
// Revision 1.11  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.10  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.9  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.7  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef    quantlib_geometric_asian_option_pricer_h
#define    quantlib_geometric_asian_option_pricer_h

#include "ql/Pricers/europeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        //! geometric Asian option
        class GeometricAsianOption : public EuropeanOption    {
           public:
            GeometricAsianOption(Type type, double underlying, double    strike,
                Rate dividendYield, Rate riskFreeRate, Time    residualTime,
                double volatility);
            double vega() const;
            double rho() const;
            Handle<SingleAssetOption> clone() const;
        };


        // inline definitions
        
        inline GeometricAsianOption::GeometricAsianOption(Type type,
            double underlying, double strike, Rate dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility):
            EuropeanOption(type, underlying, strike, dividendYield/2,
            riskFreeRate/2-volatility*volatility/12, residualTime,
            volatility/QL_SQRT(3)){}

        inline double GeometricAsianOption::rho() const{
            return EuropeanOption::rho()/2;
        }

        inline double GeometricAsianOption::vega() const{
            return EuropeanOption::vega()/QL_SQRT(3)
                -EuropeanOption::rho()*volatility_*volatility_/4;
        }

        inline Handle<SingleAssetOption> GeometricAsianOption::clone() const{
            return Handle<SingleAssetOption>(new GeometricAsianOption(*this));
        }

    }

}


#endif
