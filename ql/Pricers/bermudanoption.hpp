
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

/*! \file bermudanoption.hpp

    \fullpath
    Include/ql/Pricers/%bermudanoption.hpp
    \brief Finite-difference evaluation of Bermudan option

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:04:01  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.15  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.14  2001/08/28 12:31:45  nando
// nothing relevant (spaces added)
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
// Revision 1.8  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.7  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.6  2001/06/05 12:45:27  nando
// R019-branch-merge4 merged into trunk
//

#ifndef quantlib_bermudan_option_pricer_h
#define quantlib_bermudan_option_pricer_h

#include "ql/Pricers/multiperiodoption.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Bermudan option
        class BermudanOption : public MultiPeriodOption {
          public:
            // constructor
            BermudanOption(Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<Time>& dates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);
            Handle<SingleAssetOption> clone() const;
          protected:
            double extraTermInBermudan ;
            void initializeStepCondition() const;
            void executeIntermediateStep(int ) const;
        };

    }

}


#endif
