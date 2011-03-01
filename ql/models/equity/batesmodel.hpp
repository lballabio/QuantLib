/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file batesmodel.hpp
    \brief extended versions of the Heston model
*/

#ifndef quantlib_bates_model_hpp
#define quantlib_bates_model_hpp

#include <ql/processes/batesprocess.hpp>
#include <ql/models/equity/hestonmodel.hpp>

namespace QuantLib {

    //! Bates stochastic-volatility model
    /*! extended versions of Heston model for the stochastic
        volatility of an asset including jumps.

        References:
        A. Sepp, Pricing European-Style Options under Jump Diffusion
        Processes with Stochastic Volatility: Applications of Fourier
        Transform (<http://math.ut.ee/~spartak/papers/stochjumpvols.pdf>)

        \test calibration is tested against known values.
    */
    class BatesModel : public HestonModel {
      public:
        BatesModel(const boost::shared_ptr<BatesProcess> & process);

        Real nu()     const { return arguments_[5](0.0); }
        Real delta()  const { return arguments_[6](0.0); }
        Real lambda() const { return arguments_[7](0.0); }

      protected:
        void generateArguments();
    };


    class BatesDetJumpModel : public BatesModel {
      public:
        BatesDetJumpModel(
            const boost::shared_ptr<BatesProcess> & process,
            Real kappaLambda = 1.0, Real thetaLambda = 0.1);

        Real kappaLambda() const { return arguments_[8](0.0); }
        Real thetaLambda() const { return arguments_[9](0.0); }
    };


    class BatesDoubleExpModel : public HestonModel {
      public:
        BatesDoubleExpModel(const boost::shared_ptr<HestonProcess> & process,
                            Real lambda = 0.1,
                            Real nuUp = 0.1, Real nuDown = 0.1, Real p = 0.5);

        Real p()      const { return arguments_[5](0.0); }
        Real nuDown() const { return arguments_[6](0.0); }
        Real nuUp()   const { return arguments_[7](0.0); }
        Real lambda() const { return arguments_[8](0.0); }
    };


    class BatesDoubleExpDetJumpModel : public BatesDoubleExpModel {
      public:
        BatesDoubleExpDetJumpModel(
            const boost::shared_ptr<HestonProcess> & process,
            Real lambda = 0.1, Real nuUp = 0.1,   Real nuDown = 0.1,
            Real p = 0.5, Real kappaLambda = 1.0, Real thetaLambda = 0.1);

        Real kappaLambda() const { return arguments_[9](0.0); }
        Real thetaLambda() const { return arguments_[10](0.0); }
    };

}


#endif

