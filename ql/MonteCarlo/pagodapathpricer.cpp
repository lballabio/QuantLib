
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

/*! \file pagodapathpricer.cpp
    \brief path pricer for pagoda options

    \fullpath
    MonteCarlo/%pagodapathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/pagodapathpricer.hpp"
#include "ql/dataformatters.hpp"
#include <iostream>

namespace QuantLib {

    namespace MonteCarlo {

        PagodaPathPricer::PagodaPathPricer(const Array &underlying,
            double roof, double discount)
        : underlying_(underlying), roof_(roof), discount_(discount) {
            isInitialized_ = true;
        }

        double PagodaPathPricer::operator()(const MultiPath & multiPath) const {
            unsigned int numAssets = multiPath.assetNumber();
            unsigned int numSteps = multiPath.pathSize();
            QL_REQUIRE(isInitialized_,
                "PagodaPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "PagodaPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double averageGain = 0.0;
            for(unsigned int i = 0; i < numSteps; i++)
                for(unsigned int j = 0; j < numAssets; j++) {
                    averageGain += underlying_[j] * (QL_EXP(multiPath[j].drift()[i]+
                                    multiPath[j].diffusion()[i])-1.0);

                }

            return discount_ * QL_MAX(0.0, QL_MIN(roof_, averageGain));
        }

    }

}
