
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

/*! \file pagodapathpricer.cpp
    \brief path pricer for pagoda options

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.8  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.7  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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

        double PagodaPathPricer::value(const MultiPath & path) const {
            int numAssets = path.rows(), numSteps = path.columns();
            QL_REQUIRE(isInitialized_,
                "PagodaPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "PagodaPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double averageGain = 0.0;
            for(int j = 0; j < numSteps; j++)
                for(int i = 0; i < numAssets; i++)
                    averageGain += underlying_[i] * (QL_EXP(path[i][j])-1.0);

            return discount_ * QL_MAX(0.0, QL_MIN(roof_, averageGain));
        }

    }

}
