
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file geometricasopathpricer.hpp
    \brief path pricer for geometric average strike option
*/

#ifndef quantlib_montecarlo_geometric_average_strike_option_path_pricer_h
#define quantlib_montecarlo_geometric_average_strike_option_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! %path pricer for geometric average strike option
        class GeometricASOPathPricer_old : public PathPricer_old<Path> {
          public:
            GeometricASOPathPricer_old(Option::Type type,
                                   double underlying,
                                   DiscountFactor discount,
                                   bool useAntitheticVariance);
            double operator()(const Path& path) const;
          private:
            Option::Type type_;
            double underlying_;
        };

    }

}


#endif
