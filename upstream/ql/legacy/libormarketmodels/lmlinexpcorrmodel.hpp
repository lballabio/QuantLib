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

/*! \file lmlinexpcorrmodel.hpp
    \brief exponential correlation model for libor market models
*/

#ifndef quantlib_libor_forward_linear_exp_correlation_model_hpp
#define quantlib_libor_forward_linear_exp_correlation_model_hpp

#include <ql/legacy/libormarketmodels/lmcorrmodel.hpp>

namespace QuantLib {

    //! %linear exponential correlation model
    /*! This class describes a exponential correlation model

        \f[
        \rho_{i,j}=rho + (1-rho)*e^{(-\beta \|i-j\|)}
        \f]

        References:

        Damiano Brigo, Fabio Mercurio, Massimo Morini, 2003,
        Different Covariance Parameterizations of Libor Market Model and Joint
        Caps/Swaptions Calibration,
        (<http://www.business.uts.edu.au/qfrc/conferences/qmf2001/Brigo_D.pdf>)
    */
    class LmLinearExponentialCorrelationModel : public LmCorrelationModel {
      public:
        LmLinearExponentialCorrelationModel(Size size, Real rho, Real beta,
                                            Size factors = Null<Size>());

        Matrix correlation(Time t, const Array& x = {}) const override;
        Matrix pseudoSqrt(Time t, const Array& x = {}) const override;
        Real correlation(Size i, Size j, Time t, const Array& x) const override;

        Size factors() const override;
        bool isTimeIndependent() const override;

      protected:
        void generateArguments() override;

      private:
        Matrix corrMatrix_, pseudoSqrt_;
        const Size factors_;
    };

}


#endif

