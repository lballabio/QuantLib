
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

/*! \file geometricasianpathpricer.hpp

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_geometric_asian_pricer_h
#define quantlib_montecarlo_geometric_asian_pricer_h

#include "ql/MonteCarlo/europeanpathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! path pricer for geometric Asian options
        class GeometricAsianPathPricer : public EuropeanPathPricer {
          public:
            GeometricAsianPathPricer() {}
            GeometricAsianPathPricer(Option::Type type, double underlying,
                double strike, double discount);
            virtual double value(const Path &path) const;
        };

    }

}


#endif
