/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file analyticptdhestonengine.hpp
    \brief analytic piecewise time dependent Heston-model engine
*/

#ifndef quantlib_analytic_piecewise_time_dependent_heston_engine_hpp
#define quantlib_analytic_piecewise_time_dependent_heston_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/models/equity/piecewisetimedependenthestonmodel.hpp>


namespace QuantLib {

    //! analytic piecewise constant time dependent Heston-model engine

    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.

        J. Gatheral, The Volatility Surface: A Practitioner's Guide,
        Wiley Finance

        A. Elices, Models with time-dependent parameters using 
        transform methods: application to Heston’s model,
        http://arxiv.org/pdf/0708.2020

        \ingroup vanillaengines
    */
    class AnalyticPTDHestonEngine
        : public GenericModelEngine<PiecewiseTimeDependentHestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        // Simple to use constructor: Using adaptive
        // Gauss-Lobatto integration and Gatheral's version of complex log.
        // Be aware: using a too large number for maxEvaluations might result
        // in a stack overflow as the Lobatto integration is a recursive
        // algorithm.
        AnalyticPTDHestonEngine(
            const boost::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
            Real relTolerance, Size maxEvaluations);

        // Constructor using Laguerre integration
        // and Gatheral's version of complex log.
        AnalyticPTDHestonEngine(
            const boost::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
            Size integrationOrder = 144);

        void calculate() const;

      private:
        class Fj_Helper;
        
        const boost::shared_ptr<AnalyticHestonEngine::Integration> integration_;
    };
}

#endif
