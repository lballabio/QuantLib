
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

/*! \file basketpathpricer.hpp
    \brief multipath pricer for European-type basket option
*/

#ifndef quantlib_basket_path_pricer_h
#define quantlib_basket_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/multipath.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    //! multipath pricer for European-type basket option
    /*! The value of the option at expiration is given by the value
        of the underlying which has best performed.
    */
    class BasketPathPricer_old : public PathPricer_old<MultiPath> {
      public:
        BasketPathPricer_old(Option::Type type,
                             const std::vector<double>& underlying,
                             double strike,
                             DiscountFactor discount,
                             bool useAntitheticVariance);
        double operator()(const MultiPath& multiPath) const;
      private:
        std::vector<double> underlying_;
        // it would be easy to generalize to more exotic payoffs
        PlainVanillaPayoff payoff_;
    };

}


#endif
