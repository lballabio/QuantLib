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

/*! \file lfmcovarparam.hpp
    \brief volatility & correlation function for libor forward model process
*/

#ifndef quantlib_libor_market_covariance_parameterization_hpp
#define quantlib_libor_market_covariance_parameterization_hpp

#include <ql/math/matrix.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! %Libor market model parameterization
    /*! Brigo, Damiano, Mercurio, Fabio, Morini, Massimo, 2003,
        Different Covariance  Parameterizations of the Libor Market Model
        and Joint Caps/Swaptions Calibration
      (<http://www.exoticderivatives.com/Files/Papers/brigomercuriomorini.pdf>)
    */

    class LfmCovarianceParameterization {
      public:
        LfmCovarianceParameterization(Size size, Size factors)
        : size_(size), factors_(factors) {}
        virtual ~LfmCovarianceParameterization() = default;

        Size size() const { return size_; }
        Size factors() const { return factors_; }

        virtual Disposable<Matrix> diffusion(
                            Time t, const Array& x = Null<Array>()) const = 0;
        virtual Disposable<Matrix> covariance(
                            Time t, const Array& x = Null<Array>()) const;
        virtual Disposable<Matrix> integratedCovariance(
                            Time t, const Array& x = Null<Array>()) const;

      protected:
        const Size size_;
        const Size factors_;

      private:
        class Var_Helper;
    };

}


#endif

