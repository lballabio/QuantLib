
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

    \fullpath
    Include/ql/Pricers/%dividendeuropeanoption.hpp
    \brief european option with discrete deterministic dividends

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:04:01  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.17  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.16  2001/08/28 12:31:33  nando
// unsigned int instead of int
//
// Revision 1.15  2001/08/21 10:47:26  nando
// "dividendRho not implemented yet" moved up in the class hierarchy
//
// Revision 1.14  2001/08/13 15:06:17  nando
// added dividendRho method
//
// Revision 1.13  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.12  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.11  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.10  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.9  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.8  2001/07/05 12:35:09  enri
// - added some static_cast<int>() to prevent gcc warnings
// - added some virtual constructor (same reason)
//
// Revision 1.7  2001/07/02 12:36:18  sigmud
// pruned redundant header inclusions
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

#include "ql/Pricers/europeanoption.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        //! European option with dividends
        class DividendEuropeanOption : public EuropeanOption    {
          public:
            DividendEuropeanOption(Type type, double underlying, double strike,
                Rate dividendYield, Rate riskFreeRate, Time residualTime,
                double volatility, const std::vector<double>& dividends,
                const std::vector<Time>& exdivdates);
            double theta() const;
            double rho() const;
            double dividendRho() const {
                throw Error("DividendEuropeanOption::dividendRho not"
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
        
        inline double DividendEuropeanOption::riskless(Rate r,
            std::vector<double> divs, std::vector<Time> divDates) const{

            double tmp_riskless = 0.0;
            for(unsigned int j = 0; j < divs.size(); j++)
                tmp_riskless += divs[j]*QL_EXP(-r*divDates[j]);
            return tmp_riskless;
        }

    }

}


#endif
