/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file roughhestonmodel.hpp
    \brief rough Heston model for the stochastic volatility of an asset
*/

#ifndef quantlib_rough_heston_model_hpp
#define quantlib_rough_heston_model_hpp

#include <ql/models/model.hpp>
#include <ql/processes/hestonprocess.hpp>

namespace QuantLib {

    //! rough Heston model for the stochastic volatility of an asset
    /*! References:

        O. El Euch and M. Rosenbaum, The characteristic function of rough
        Heston models, Mathematical Finance 29(1), 3-38 (2019).

        J. Gatheral, T. Jaisson and M. Rosenbaum, Volatility is rough,
        Quantitative Finance 18(6), 933-949 (2018).

        \test calibration is tested against known synthetic parameters.
    */
    class RoughHestonModel : public CalibratedModel {
      public:
        RoughHestonModel(const ext::shared_ptr<HestonProcess>& process,
                         Real hurst);

        //! variance mean-reversion level
        Real theta() const { return arguments_[0](0.0); }
        //! variance mean-reversion speed
        Real kappa() const { return arguments_[1](0.0); }
        //! volatility of the volatility
        Real sigma() const { return arguments_[2](0.0); }
        //! correlation
        Real rho()   const { return arguments_[3](0.0); }
        //! spot variance
        Real v0()    const { return arguments_[4](0.0); }
        //! Hurst exponent of the variance process
        Real hurst() const { return arguments_[5](0.0); }

        ext::shared_ptr<HestonProcess> process() const { return process_; }

      private:
        void generateArguments() override;
        ext::shared_ptr<HestonProcess> process_;
    };

}

#endif
