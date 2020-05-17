/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Klaus Spanderen

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

/*! \file exponentialfittinghestonengine.hpp
    \brief analytic Heston-model engine based on exponential fitting
*/

#ifndef quantlib_exponential_fitting_heston_engine_hpp
#define quantlib_exponential_fitting_heston_engine_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>

#include <vector>

namespace QuantLib {

    //! analytic Heston-model engine based on
    //  exponentially fitted Gauss-Laguerre quadrature

    /*! References:
        D. Conte, L. Ixaru, B. Paternoster, G. Santomauro, 2014
        Exponentially-fitted Gauss–Laguerre quadrature rule for
        integrals over an unbounded interval

        For adaptation details see
        https://wordpress.com/post/hpcquantlib.wordpress.com/4557
    */

    class ExponentialFittingHestonEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        explicit ExponentialFittingHestonEngine(
            const ext::shared_ptr<HestonModel>& model,
            Real scaling = Null<Real>());

        void calculate() const;

      private:
        const Real scaling_;
        static std::vector<Real> moneyness_;
    };
}

#endif
