/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file lmextlinexpvolmodel.hpp
    \brief volatility model for libor market models
*/

#ifndef quantlib_libor_market_ext_linear_exp_volatility_model_hpp
#define quantlib_libor_market_ext_linear_exp_volatility_model_hpp

#include <ql/legacy/libormarketmodels/lmlinexpvolmodel.hpp>

namespace QuantLib {

    //! extended linear exponential volatility model
    /*! This class describes an extended linear-exponential volatility model

        \f[
        \sigma_i(t)=k_i*((a*(T_{i}-t)+d)*e^{-b(T_{i}-t)}+c)
        \f]

        References:

        Damiano Brigo, Fabio Mercurio, Massimo Morini, 2003,
        Different Covariance Parameterizations of Libor Market Model and Joint
        Caps/Swaptions Calibration,
        (<http://www.business.uts.edu.au/qfrc/conferences/qmf2001/Brigo_D.pdf>)
    */

    class LmExtLinearExponentialVolModel
        : public LmLinearExponentialVolatilityModel {
      public:
        LmExtLinearExponentialVolModel(const std::vector<Time>& fixingTimes,
                                       Real a, Real b, Real c, Real d);

        Array volatility(Time t, const Array& x = {}) const override;
        Volatility volatility(Size i, Time t, const Array& x = {}) const override;

        Real integratedVariance(Size i, Size j, Time u, const Array& x = {}) const override;
    };

}


#endif

