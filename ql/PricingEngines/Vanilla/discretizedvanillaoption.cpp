
/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb

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

/*! \file discretizedvanillaoption.cpp
    \brief discretized vanilla option
*/

#include <ql/PricingEngines/Vanilla/discretizedvanillaoption.hpp>
#include <vector>

namespace QuantLib {

    void DiscretizedVanillaOption::reset(Size size) {
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedVanillaOption::postAdjustValues() {

        Time now = time();
        Size i;
        switch(arguments_.exercise->type()) {
          case Exercise::American:
            if (now <= arguments_.stoppingTimes[1] && 
                now >= arguments_.stoppingTimes[0])
                applySpecificCondition();
            break;
          case Exercise::European:
            if (isOnTime(arguments_.stoppingTimes[0]))
                applySpecificCondition();
            break;
          case Exercise::Bermudan:
            for (i = 0; i<arguments_.stoppingTimes.size(); i++) {
                if (isOnTime(arguments_.stoppingTimes[i]))
                    applySpecificCondition();
            }
            break;
          default:
            QL_FAIL("DiscretizedVanillaOption::postAdjustValues() : "
                    "invalid option type");
        }
    }

    void DiscretizedVanillaOption::applySpecificCondition() {
        #if defined(HAVE_BOOST)
        Handle<BlackScholesLattice> lattice = 
            boost::dynamic_pointer_cast<BlackScholesLattice>(method());
        QL_REQUIRE(lattice,
                   "DiscretizedVanillaOption: "
                   "non-Black-Scholes lattice given");
        #else
        Handle<BlackScholesLattice> lattice = method();
        #endif
        Handle<Tree> tree(lattice->tree());
        Size i = method()->timeGrid().findIndex(time());

        for (Size j=0; j<values_.size(); j++) {
            values_[j] = 
                QL_MAX(values_[j],
                       (*arguments_.payoff)(tree->underlying(i, j)));
        }
    }

}

