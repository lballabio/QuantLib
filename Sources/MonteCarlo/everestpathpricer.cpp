
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

/*! \file everestpathpricer.cpp
    \fullpath Sources/MonteCarlo/%everestpathpricer.cpp
    \brief path pricer for European-type Everest option

*/

// $Id$
// $Log$
// Revision 1.14  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.13  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.9  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.8  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/everestpathpricer.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        EverestPathPricer::EverestPathPricer(double discount):
            discount_(discount) {
            QL_REQUIRE(discount_ > 0.0,
                "EverestPathPricer: discount must be positive");
            isInitialized_ = true;
        }

        double EverestPathPricer::value(const MultiPath & path) const {
            int numAssets = path.rows();
            QL_REQUIRE(isInitialized_,
                "EverestPathPricer: pricer not initialized");

            double minPrice = QL_MAX_DOUBLE;
            for(int i = 0; i < numAssets; i++)
                minPrice = QL_MIN(minPrice, QL_EXP(path[i][0]));

            return discount_ * minPrice;
        }

    }

}
