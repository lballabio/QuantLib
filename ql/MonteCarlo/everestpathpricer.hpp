
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

/*! \file everestpathpricer.hpp
    \brief path pricer for European-type Everest option

    \fullpath
    ql/MonteCarlo/%everestpathpricer.hpp

*/

// $Id$

#ifndef quantlib_everest_path_pricer_h
#define quantlib_everest_path_pricer_h

#include <vector>
#include "ql/MonteCarlo/multipathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! %path pricer for European-type Everest option
        /*! The payoff of an Everest option is simply given by the
            final-price initial-price ratio of the worst performer.
        */
        class EverestPathPricer : public MultiPathPricer {
          public:
            EverestPathPricer():MultiPathPricer(){}
            EverestPathPricer(double discount);
            double operator()(const MultiPath &path) const;
          protected:
            double discount_;
        };

    }

}


#endif
