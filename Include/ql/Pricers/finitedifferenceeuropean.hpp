
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

/*! \file finitedifferenceeuropean.hpp

    \fullpath
    Include/ql/Pricers/%finitedifferenceeuropean.hpp
    \brief Example of European option calculated using finite differences

*/

// $Id$
// $Log$
// Revision 1.13  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.12  2001/08/28 14:47:46  nando
// unsigned int instead of int
//
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
// Revision 1.4  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_pricers_finite_difference_european_option_h
#define quantlib_pricers_finite_difference_european_option_h

#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Example of European option calculated using finite differences
        class FiniteDifferenceEuropean : public BSMNumericalOption {
          public:
            FiniteDifferenceEuropean(Type type,
                                     double underlying,
                                     double strike,
                                     Rate dividendYield,
                                     Rate riskFreeRate,
                                     Time residualTime,
                                     double volatility,
                                     unsigned int timeSteps = 200,
                                     unsigned int gridPoints = 800);
           	Array getPrices() const;
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(new FiniteDifferenceEuropean(*this));
            }
          protected:
            void calculate() const;
          private:
            unsigned int timeSteps_;
            mutable Array euroPrices_;
        };

        // inline definitions
        
        inline Array  FiniteDifferenceEuropean::getPrices() const{
            value();
            return euroPrices_;
        }

    }

}

#endif
