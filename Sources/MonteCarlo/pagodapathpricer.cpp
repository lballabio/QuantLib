/*
 * Copyright (C) 2001
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
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file pagodapathpricer.cpp

    $Source$
    $Log$
    Revision 1.1  2001/03/22 12:12:52  marmar
    Introducing pagoda options

*/

#include "pagodapathpricer.h"
#include "qlerrors.h"
#include "dataformatters.h"
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
