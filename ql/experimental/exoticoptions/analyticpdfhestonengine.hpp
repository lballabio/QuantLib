/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file analyticpdfhestonengine.hpp
    \brief Analytic engine for arbitrary European payoffs under the Heston model
*/

#ifndef quantlib_analytic_pdf_heston_engine_hpp
#define quantlib_analytic_pdf_heston_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>

namespace QuantLib {

    //! Analytic engine for arbitrary European payoffs under the Heston model

    /*! References:

        The formulas are taken from A. Dragulescu, V. Yakovenko, 2002.
        Probability distribution of returns in the Heston model
        with stochastic volatility.
        http://arxiv.org/pdf/cond-mat/0203046.pdf

        \test the correctness of the returned value is tested by
              reproducing digital prices using call spreads and the
              AnalyticHestonEngine.

    */
    class AnalyticPDFHestonEngine
       : public GenericModelEngine<HestonModel,
                                   VanillaOption::arguments,
                                   VanillaOption::results> {
      public:
        AnalyticPDFHestonEngine(
            const boost::shared_ptr<HestonModel>& model,
            Real eps = 1e-6,
            Size nIterations = 100000ul,
            Real xMax = Null<Real>());
        void calculate() const;

        // reduced probability,
        // hidden variable \nu is integrated out.
        Real Pv(Real s_0, Real s_t, Time t) const;

        // reduced probability in x_t = ln(s_t/s_0) - (r-q)*t
        Real Pv(Real x_t, Time t) const;

      private:
        Real weightedPayoff(Real x_t, Time t) const;

        const Real eps_, xMax_;
        const Size nIterations_;

        const boost::shared_ptr<HestonModel> model_;
    };
}


#endif
