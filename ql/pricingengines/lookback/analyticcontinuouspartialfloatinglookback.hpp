/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2014 Francois Botha

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

/*! \file analyticcontinuousfloatinglookback.hpp
    \brief Analytic engine for continuous floating-strike lookback
*/

#ifndef quantlib_analytic_continuous_partial_floating_lookback_engine_hpp
#define quantlib_analytic_continuous_partial_floating_lookback_engine_hpp

#include <ql/instruments/lookbackoption.hpp>
#include <ql/pricingengines/lookback/analyticcontinuousfloatinglookback.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>


namespace QuantLib {

    //! Pricing engine for European continuous partial-time floating-strike lookback option
    /*! Formula from "Option Pricing Formulas, Second Edition",
        E.G. Haug, 2006, p.146

        \ingroup lookbackengines

        \test returned values verified against results from literature
    */
    class AnalyticContinuousPartialFloatingLookbackEngine
        : public ContinuousPartialFloatingLookbackOption::engine {
      public:
        AnalyticContinuousPartialFloatingLookbackEngine(
            std::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        std::shared_ptr<GeneralizedBlackScholesProcess> process_;
        CumulativeNormalDistribution f_;
        // helper methods
        Real underlying() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real minmax() const;
        Real lambda() const;
        Time lookbackPeriodEndTime() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real A(Real eta) const;
    };

}


#endif
