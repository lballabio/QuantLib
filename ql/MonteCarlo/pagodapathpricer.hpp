
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

/*! \file pagodapathpricer.hpp
    \brief path pricer for pagoda options

    \fullpath
    ql/MonteCarlo/%pagodapathpricer.hpp
*/

// $Id$

#ifndef quantlib_pagoda_path_pricer_h
#define quantlib_pagoda_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/multipath.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! %multipath pricer for pagoda options
        /*! A pagoda option is a multi-asset asian option with a cap
            (the pagoda "roof").
            Given a portfolio of assets the payoff is the
            arithmetic average of the portfolio performance,
            with a maximum cap given by the roof.
        */
        class PagodaPathPricer : public PathPricer<MultiPath> {
          public:
            PagodaPathPricer(const Array& underlying,
                             double roof,
                             DiscountFactor discount,
                             bool useAntitheticVariance);
            double operator()(const MultiPath& path) const;
          private:
            Array underlying_;
            double roof_;
        };

    }

}

#endif
