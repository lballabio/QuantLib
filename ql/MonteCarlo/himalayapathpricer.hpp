
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

/*! \file himalayapathpricer.hpp
    \brief multipath pricer for European-type Himalaya option

    \fullpath
    ql/MonteCarlo/%himalayapathpricer.hpp

*/

// $Id$

#ifndef quantlib_himalaya_path_pricer_h
#define quantlib_himalaya_path_pricer_h


#include "ql/MonteCarlo/multipathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! multipath pricer for European-type Himalaya option
        /*! The payoff of an himalaya option is computed in the following way:
            given a basket of N assets, and M time periods, at end of
            each period the option who performed the best is added to the
            average and then discarded from the basket. At the end of the
            M periods the option pays the max between the strike and the
            average of the best performers.
        */
        class HimalayaPathPricer : public MultiPathPricer {
          public:
//            HimalayaPathPricer() {}
            HimalayaPathPricer(const Array& underlying,
                               double strike,
                               double discount,
                               bool antitheticVariance);
            double operator()(const MultiPath& multiPath) const;
          protected:
            Array underlying_;
            double strike_;
            double discount_;
            bool antitheticVariance_;
        };

    }

}

#endif
