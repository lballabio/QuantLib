
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

#include <ql/MonteCarlo/pagodapathpricer.hpp>
#include <ql/dataformatters.hpp>
#include <iostream>

namespace QuantLib {

    namespace MonteCarlo {

        PagodaPathPricer::PagodaPathPricer(const Array& underlying,
            double roof,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying), roof_(roof) {}

        double PagodaPathPricer::operator()(const MultiPath& multiPath) const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "PagodaPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");


            Size i,j;
            if (useAntitheticVariance_) {
                double averageGain = 0.0, averageGain2 = 0.0;
                for(i = 0; i < numSteps; i++)
                    for(j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]+
                                    multiPath[j].diffusion()[i])
                             -1.0);
                        averageGain2 += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]-
                                    multiPath[j].diffusion()[i])
                             -1.0);
                    }
                return discount_ * 0.5 *
                    (QL_MAX(0.0, QL_MIN(roof_, averageGain))+
                     QL_MAX(0.0, QL_MIN(roof_, averageGain2)));
            } else {
                double averageGain = 0.0;
                for(i = 0; i < numSteps; i++)
                    for(j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]+
                                    multiPath[j].diffusion()[i])
                             -1.0);
                    }
                return discount_ * QL_MAX(0.0, QL_MIN(roof_, averageGain));
            }

        }

    }

}
