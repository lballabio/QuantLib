/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file analytichestonhullwhiteengine.hpp
    \brief analytic heston engine incl. stochastic interest rates
*/

#ifndef quantlib_analytic_heston_hull_white_engine_hpp
#define quantlib_analytic_heston_hull_white_engine_hpp

#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

namespace QuantLib {

    //! Analytic Heston engine incl. stochastic interest rates
    /*! This class is pricing a european option under the following process

        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d) S dt +\sqrt{v} S dW_1 \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        dr(t)     &=& (\theta(t) - a r) dt + \eta dW_3 \\
        dW_1 dW_2 &=& \rho dt \\
        dW_1 dW_3 &=& 0 \\
        dW_2 dW_3 &=& 0 \\
        \end{array}
        \f]

        References:

        Karel in't Hout, Joris Bierkens, Antoine von der Ploeg,
        Joe in't Panhuis, A Semi closed-from analytic pricing formula for
        call options in a hybrid Heston-Hull-White Model.

        A. Sepp, Pricing European-Style Options under Jump Diffusion
        Processes with Stochastic Volatility: Applications of Fourier
        Transform (<http://math.ut.ee/~spartak/papers/stochjumpvols.pdf>)

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature, testing
              against QuantLib's analytic Heston and
              Black-Scholes-Merton Hull-White engine
    */
    class AnalyticHestonHullWhiteEngine : public AnalyticHestonEngine {
      public:
        // see AnalticHestonEninge for usage of different constructors
        AnalyticHestonHullWhiteEngine(const ext::shared_ptr<HestonModel>& hestonModel,
                                      ext::shared_ptr<HullWhite> hullWhiteModel,
                                      Size integrationOrder = 144);

        AnalyticHestonHullWhiteEngine(const ext::shared_ptr<HestonModel>& model,
                                      ext::shared_ptr<HullWhite> hullWhiteModel,
                                      Real relTolerance,
                                      Size maxEvaluations);


        void update() override;
        void calculate() const override;

      protected:
        std::complex<Real> addOnTerm(Real phi, Time t, Size j) const override;

        const ext::shared_ptr<HullWhite> hullWhiteModel_;

      private:
        mutable Real m_;
        mutable Real a_, sigma_;
    };

    inline
    std::complex<Real> AnalyticHestonHullWhiteEngine::addOnTerm(Real u,
                                                                Time,
                                                                Size j) const {
        return std::complex<Real>(-m_*u*u, u*(m_-2*m_*(j-1)));
    }

}

#endif
