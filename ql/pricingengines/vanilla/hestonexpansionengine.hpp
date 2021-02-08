/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Fabien Le Floc'h

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

/*! \file analytichestonengine.hpp
    \brief analytic Heston expansion engine
*/

#ifndef quantlib_heston_expansion_engine_hpp
#define quantlib_heston_expansion_engine_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>

namespace QuantLib {

    //! Heston-model engine for European options based on analytic expansions
    /*! References:

        M Forde, A Jacquier, R Lee, The small-time smile and term
        structure of implied volatility under the Heston model
        SIAM Journal on Financial Mathematics, 2012 - SIAM

        M Lorig, S Pagliarani, A Pascucci, Explicit implied vols for
        multifactor local-stochastic vol models
        arXiv preprint arXiv:1306.5447v3, 2014 - arxiv.org

        \ingroup vanillaengines
    */
    class HestonExpansionEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        enum HestonExpansionFormula { LPP2, LPP3, Forde };

        HestonExpansionEngine(const ext::shared_ptr<HestonModel>& model,
                              HestonExpansionFormula formula);

        void calculate() const override;

      private:
        const HestonExpansionFormula formula_;
    };

    /*! Interface to represent some Heston expansion formula.
        During calibration, it would typically be initialized once per
        implied volatility surface slice, then calls for each surface
        strike to impliedVolatility(strike, forward) would be
        performed.
    */
    class HestonExpansion {
      public:
        virtual ~HestonExpansion() = default;
        virtual Real impliedVolatility(Real strike, Real forward) const = 0;
    };

    /*! Lorig Pagliarani Pascucci expansion of order-2 for the Heston model.
        During calibration, it can be initialized once per expiry, and
        called many times with different strikes.  The formula is also
        available in the Mathematica notebook from the authors at
        http://explicitsolutions.wordpress.com/
    */
    class LPP2HestonExpansion : public HestonExpansion {
      public:
        LPP2HestonExpansion(Real kappa, Real theta, Real sigma, Real v0, Real rho, Real term);
        Real impliedVolatility(Real strike, Real forward) const override;

      private:
        Real coeffs[3];
        Real ekt, e2kt, e3kt, e4kt;
        Real z0(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
        Real z1(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
        Real z2(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
    };

    /*! Lorig Pagliarani Pascucci expansion of order-3 for the Heston model.
        During calibration, it can be initialized once per expiry, and
        called many times with different strikes.  The formula is also
        available in the Mathematica notebook from the authors at
        http://explicitsolutions.wordpress.com/
    */
    class LPP3HestonExpansion : public HestonExpansion{
      public:
        LPP3HestonExpansion(Real kappa, Real theta, Real sigma, Real v0, Real rho, Real term);
        Real impliedVolatility(Real strike, Real forward) const override;

      private:
        Real coeffs[4];
        Real ekt, e2kt, e3kt, e4kt;
        Real z0(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
        Real z1(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
        Real z2(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
        Real z3(Real t, Real kappa, Real theta,
                Real delta, Real y, Real rho) const;
    };

    /*! Small-time expansion from
        "The small-time smile and term structure of implied volatility
        under the Heston model" M Forde, A Jacquier, R Lee - SIAM
        Journal on Financial Mathematics, 2012 - SIAM
    */
    class FordeHestonExpansion : public HestonExpansion {
      public:
        FordeHestonExpansion(Real kappa, Real theta, Real sigma, Real v0, Real rho, Real term);
        Real impliedVolatility(Real strike, Real forward) const override;

      private:
        Real coeffs[5];
    };

}


#endif
