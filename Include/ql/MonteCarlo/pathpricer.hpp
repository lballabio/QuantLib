
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

/*! \file pathpricer.hpp

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.5  2001/01/24 13:14:25  marmar
    Removed typedef

    Revision 1.4  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.3  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.2  2001/01/05 11:18:03  lballabio
    Renamed SinglePathGeometricAsianPricer to GeometricAsianPathPricer

    Revision 1.1  2001/01/05 11:02:37  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_montecarlo_path_pricer_h
#define quantlib_montecarlo_path_pricer_h

#include "ql/qldefines.hpp"
#include "ql/MonteCarlo/path.hpp"

namespace QuantLib {

    namespace MonteCarlo {

    //! base class for single-path pricers
    /*! PathPricer is the base class for an hierarchy of single-path pricers.
        Given a path the value of an option is returned on that path.
    */

        class PathPricer {
        public:
            PathPricer() : isInitialized_(false) {}
            virtual ~PathPricer() {}
            virtual double value(const Path &path) const=0;
        protected:
            bool isInitialized_;
        };

    }

}


#endif
