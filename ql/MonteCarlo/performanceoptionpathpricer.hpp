
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file performanceoptionpathpricer.hpp
    \brief path pricer for performance option
*/

// $Id$

#ifndef quantlib_montecarlo_performanceoption_path_pricer_h
#define quantlib_montecarlo_performanceoption_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! %path pricer for performance options
        class PerformanceOptionPathPricer_old : public PathPricer_old<Path> {
          public:
            PerformanceOptionPathPricer_old(Option::Type type,
                               double underlying,
                               double moneyness,
                               const std::vector<DiscountFactor>& discounts,
                               bool useAntitheticVariance);
            double operator()(const Path& path) const;
          private:
            double underlying_;
            std::vector<DiscountFactor> discounts_;
            // it would be easy to generalize to more exotic payoffs
            PlainPayoff payoff_;
        };

    }

}


#endif
