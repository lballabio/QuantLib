
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
        class PagodaPathPricer_old : public PathPricer_old<MultiPath> {
          public:
            PagodaPathPricer_old(const Array& underlying,
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
