/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 Klaus Spanderen

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

/*! \file analytichestonengine.hpp
    \brief analytic Heston-model engine
*/

#ifndef quantlib_analytic_heston_engine_hpp
#define quantlib_analytic_heston_engine_hpp

#include <ql/qldefines.hpp>

#if !defined(QL_PATCH_MSVC6)

#include <ql/PricingEngines/genericmodelengine.hpp>
#include <ql/ShortRateModels/TwoFactorModels/hestonmodel.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/Math/gaussianquadratures.hpp>
#include <complex>

namespace QuantLib {

    //! analytic Heston-model engine based on Fourier transform
    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.

        Dupire, Bruno, 1994. Pricing with a smile. Risk Magazine, 7, 18-20.

        A. Sepp, Pricing European-Style Options under Jump Diffusion
        Processes with Stochastic Volatility: Applications of Fourier
        Transform (<http://math.ut.ee/~spartak/papers/stochjumpvols.pdf>)

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */

    class AnalyticHestonEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        AnalyticHestonEngine(const boost::shared_ptr<HestonModel> & model,
                             Size integrationOrder = 64);
        void calculate() const;
        // call back for extended stochastic volatility
        // plus jump diffusion engines like bates model
        virtual std::complex<Real>
            jumpDiffusionTerm(Real phi, Time t, Size j) const;

      private:
        GaussLaguerreIntegration gaussLaguerre;

        class Fj_Helper;
    };

}

#endif

#endif
