
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
/*! \file path.hpp
    \fullpath Include/ql/MonteCarlo/%path.hpp
    \brief Monte Carlo path

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.7  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.6  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_path_h
#define quantlib_montecarlo_path_h

#include "ql/array.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*!
        For the time being Path is equivalent to Array.    In the future this
        could change and Path might contain more information.
        As of today, Path contains the list of continuously-compounded
        variations,
        \f[
            \log \frac{Y_{i+1}}{Y_i} \mathrm{for} i = 0, \ldots, n-1
        \f]
        where \f$ Y_i \f$ is the value of the underlying at discretized time
        \f$ t_i \f$.
        */
        typedef Array Path;
    }

}


#endif
