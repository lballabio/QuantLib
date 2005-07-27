/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hestonmodel.hpp
    \brief Heston model for the stochastic volatility of an asset
*/

#ifndef quantlib_heston_model_hpp
#define quantlib_heston_model_hpp

#include <ql/ShortRateModels/model.hpp>
#include <ql/Processes/hestonprocess.hpp>

namespace QuantLib {

    //! Heston model for the stochastic volatility of an asset
    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.

        \test calibration is tested against known good values.
    */
    class HestonModel : public ShortRateModel {
      public:
        HestonModel(const boost::shared_ptr<HestonProcess> & process);

        // variance mean version level
        Real theta() const { return arguments_[0](0.0); }
        // variance mean reversion speed
        Real kappa() const { return arguments_[1](0.0); }
        // volatility of the volatility
        Real sigma() const { return arguments_[2](0.0); }
        // correlation
        Real rho()   const { return arguments_[3](0.0); }
        // spot variance
        Real v0()    const { return arguments_[4](0.0); }

        boost::shared_ptr<NumericalMethod> tree(const TimeGrid&) const;

        class VolatilityConstraint;
    };

}


#endif

