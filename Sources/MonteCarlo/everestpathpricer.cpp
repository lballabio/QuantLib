
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file everestpathpricer.cpp

    $Sourc$
    $Log$
    Revision 1.6  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.5  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.3  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/03/07 09:33:08  marmar
    Spot prices not necessary for evaluation of everest option

    Revision 1.1  2001/03/06 16:59:31  marmar
    First, simplified version, of everest option

*/

#include "ql/MonteCarlo/everestpathpricer.hpp"
#include "ql/qlerrors.hpp"
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
