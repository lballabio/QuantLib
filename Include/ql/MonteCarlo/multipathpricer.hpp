
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

/*! \file multipathpricer.hpp
    \fullpath Include/ql/MonteCarlo/%multipathpricer.hpp
    \brief base class for multi-path pricers

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.8  2001/07/20 13:06:57  marmar
// Monte Carlo interfaces imporved
//
// Revision 1.7  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_multi_path_pricer_h
#define quantlib_montecarlo_multi_path_pricer_h

#include "ql/MonteCarlo/multipath.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! base class for multi-path pricers
        /*! Given a multi-path the value of an option is returned on
            that path.
        */
        class MultiPathPricer {
          public:
            typedef double ValueType;
            MultiPathPricer() : isInitialized_(false) {}
            virtual ~MultiPathPricer() {}
            virtual double value(const MultiPath &multiPath) const = 0;
          protected:
            bool isInitialized_;
        };

    }

}


#endif
