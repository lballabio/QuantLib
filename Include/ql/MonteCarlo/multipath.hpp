
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
/*! \file multipath.hpp
    \brief MultiPath implements multiple paths evolving at the same time

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

*/

#ifndef quantlib_montecarlo_multi_path_h
#define quantlib_montecarlo_multi_path_h

#include "ql/Math/matrix.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*!
        \brief MultiPath implements multiple paths evolving at the same time
        \typedef MultiPath
        MultiPath contains the list of variations for each asset,
        \f[
            \log \frac{Y^j_{i+1}}{Y^j_i} \mathrm{for} i = 0, \ldots, n-1
            \qquad \mathrm{and} \qquad j = 0, \ldots, m-1
        \f]
        where \f$ Y^j_i \f$ is the value of the underlying \f$ j \f$
        at discretized time \f$ t_i \f$. The first index refers to the
        underlying, the second to the time position MultiPath[j,i]
        */
        typedef QuantLib::Math::Matrix MultiPath;
    }

}

#endif
