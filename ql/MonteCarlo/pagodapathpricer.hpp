
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
