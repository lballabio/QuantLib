/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

/*! \file analyticdoublebarrierengine.hpp
    \brief Analytic double barrier european option engines

  ! Valid only if strike is in barrier range
*/

#ifndef quantlib_analytic_double_barrier_engine_hpp
#define quantlib_analytic_double_barrier_engine_hpp

#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Pricing engine for double barrier european options using analytical formulae
    /*! The formulas are taken from "The complete guide to option pricing formulas 2nd Ed",
         E.G. Haug, McGraw-Hill, p.156 and following.
         Implements the Ikeda and Kunitomo series (see "Pricing Options with 
         Curved Boundaries" Mathematical Finance 2/1992").
         This code handles only flat barriers

        \ingroup barrierengines

        \note the formula holds only when strike is in the barrier range

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class AnalyticDoubleBarrierEngine : public DoubleBarrierOption::engine {
      public:
        explicit AnalyticDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process, int series = 5);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        CumulativeNormalDistribution f_;
        int series_;
        // helper methods
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Volatility volatility() const;
        Real volatilitySquared() const;
        Real barrierLo() const;
        Real barrierHi() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        Rate costOfCarry() const;
        DiscountFactor dividendDiscount() const;
        Real vanillaEquivalent() const;
        Real callKO() const;
        Real putKO() const;
        Real callKI() const;
        Real putKI() const;
    };

}


#endif
