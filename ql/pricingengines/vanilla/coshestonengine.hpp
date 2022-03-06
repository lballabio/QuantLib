/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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

/*! \file coshestonengine.hpp
    \brief Heston engine based on Fourier-Cosine series expansions
*/

#ifndef quantlib_cos_heston_engine_hpp
#define quantlib_cos_heston_engine_hpp

#include <ql/models/equity/hestonmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>

#include <complex>

namespace QuantLib {

    //! COS-method Heston engine based on efficient Fourier series expansions

    /*! References:

        F. Fang, C.W. Oosterlee: A Novel Pricing Method for European Ooptions
        based on Fourier-Cosine Series Expansions,
        http://ta.twi.tudelft.nl/mf/users/oosterle/oosterlee/COS.pdf

        Fabien Le Floc'h: Fourier Integration and Stochastic Volatility
        Calibration,
        https://papers.ssrn.com/sol3/papers2.cfm?abstract_id=2362968

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */
    class COSHestonEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        explicit COSHestonEngine(const ext::shared_ptr<HestonModel>& model,
                                 Real L = 16, Size N=200);

        void update() override;
        void calculate() const override;

        // normalized characteristic function
        std::complex<Real> chF(Real u, Real t) const;

        Real c1(Time t) const;
        Real c2(Time t) const;
        Real c3(Time t) const;
        Real c4(Time t) const;

        Real mu(Time t) const;
        Real var(Time t) const;
        Real skew(Time t) const;
        Real kurtosis(Time t) const;

      private:
        Real muT(Time t) const;

        const Real L_;
        const Size N_;
        Real kappa_, theta_, sigma_, rho_, v0_;
    };
}

#endif


#ifndef id_ceeb5d0714f369dd2a031c04573adafc
#define id_ceeb5d0714f369dd2a031c04573adafc
inline bool test_ceeb5d0714f369dd2a031c04573adafc(const int* i) {
    return i != nullptr;
}
#endif
