
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

/*! \file basketpathpricer.hpp
    \brief multipath pricer for European-type basket option

    \fullpath
    ql/MonteCarlo/%basketpathpricer.hpp

*/

// $Id$

#ifndef quantlib_basket_path_pricer_h
#define quantlib_basket_path_pricer_h

#include "ql/MonteCarlo/multipathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! multipath pricer for European-type basket option
        /*! The value of the option at expiration is given by the value
            of the underlying which has best performed.
        */
        class BasketPathPricer : public MultiPathPricer {
          public:
            BasketPathPricer() {}
            BasketPathPricer(const Array& underlying,
                             double discount);
            double operator()(const MultiPath& path) const;
          protected:
            Array underlying_;
            double discount_;
        };

    }

}


#endif
