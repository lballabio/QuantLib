
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

/*! \file pathpricer.hpp
    \brief base class for single-path pricers

    \fullpath
    ql/MonteCarlo/%pathpricer.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_path_pricer_h
#define quantlib_montecarlo_path_pricer_h

#include <ql/option.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! base class for path pricers
        /*! Given a path the value of an option is returned on that path.
        */
        template<class P>
        class PathPricer : public std::unary_function<P, double> {
          public:
            PathPricer(DiscountFactor discount,
                       bool useAntitheticVariance);
            virtual ~PathPricer() {}
            virtual double operator()(const P& path) const=0;
          protected:
            DiscountFactor discount_;
            bool useAntitheticVariance_;
        };

        template<class P>
        PathPricer<P>::PathPricer(DiscountFactor discount,
            bool useAntitheticVariance)
            : discount_(discount),
              useAntitheticVariance_(useAntitheticVariance) {
            QL_REQUIRE(discount_ <= 1.0 && discount_ > 0.0,
                "PathPricer: discount must be positive");
        }

    }

}


#endif
