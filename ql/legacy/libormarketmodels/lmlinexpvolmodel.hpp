/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file lmlinexpvolmodel.hpp
    \brief volatility model for libor market models
*/

#ifndef quantlib_libor_market_linear_exp_volatility_model_hpp
#define quantlib_libor_market_linear_exp_volatility_model_hpp

#include <ql/legacy/libormarketmodels/lmvolmodel.hpp>

namespace QuantLib {

    //! %linear exponential volatility model
    /*! This class describes a linear-exponential volatility model

        \f[
        \sigma_i(t)=(a*(T_{i}-t)+d)*e^{-b(T_{i}-t)}+c
        \f]

        References:

        Damiano Brigo, Fabio Mercurio, Massimo Morini, 2003,
        Different Covariance Parameterizations of Libor Market Model and Joint
        Caps/Swaptions Calibration,
        (<http://www.business.uts.edu.au/qfrc/conferences/qmf2001/Brigo_D.pdf>)
    */

    class LmLinearExponentialVolatilityModel : public LmVolatilityModel {
      public:
        LmLinearExponentialVolatilityModel(
                                         const std::vector<Time>& fixingTimes,
                                         Real a, Real b, Real c, Real d);

        Array volatility(Time t, const Array& x = Null<Array>()) const override;
        Volatility volatility(Size i, Time t, const Array& x = Null<Array>()) const override;

        Real
        integratedVariance(Size i, Size j, Time u, const Array& x = Null<Array>()) const override;

      private:
        void generateArguments() override;
        const std::vector<Time> fixingTimes_;
    };

}


#endif

