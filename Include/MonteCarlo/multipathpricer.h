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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file multipathpricer.h

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/02/02 10:50:02  marmar
    Include-guard redefined

    Revision 1.1  2001/01/25 16:11:17  marmar
    MultiPath, the first step for a multi-dimensional Monte Carlo

*/

#ifndef quantlib_montecarlo_multi_path_pricer_h
#define quantlib_montecarlo_multi_path_pricer_h

#include "qldefines.h"
#include "multipath.h"

namespace QuantLib {

    namespace MonteCarlo {

    //! Base class for multi-path pricers
    /*! MultiPathPricer is the base class for an hierarchy of multi-path
        pricers. Given a multi-path the value of an option is returned on 
        that path.
    */

        class MultiPathPricer {
        public:
            MultiPathPricer() : isInitialized_(false) {}
            virtual ~MultiPathPricer() {}
            virtual double value(const MultiPath &multiPath) const=0;
        protected:
            bool isInitialized_;
        };

    }

}


#endif
