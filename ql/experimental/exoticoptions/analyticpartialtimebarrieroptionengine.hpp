/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file analyticpartialtimebarrieroptionengine.hpp
    \brief Analytic engine for partial-time barrier options
*/

#ifndef quantlib_analytic_partial_time_barrier_option_engine_hpp
#define quantlib_analytic_partial_time_barrier_option_engine_hpp

#include <ql/experimental/exoticoptions/partialtimebarrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    class AnalyticPartialTimeBarrierOptionEngine
        : public PartialTimeBarrierOption::engine {
      public:
        explicit AnalyticPartialTimeBarrierOptionEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Real calculate(PartialTimeBarrierOption::arguments& arguments,
                      ext::shared_ptr<PlainVanillaPayoff> payoff, 
                      ext::shared_ptr<GeneralizedBlackScholesProcess> process) const;
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Time coverEventTime() const;
        Volatility volatility(Time t, Real strike) const;
        Real barrier() const;
        Real rebate() const;
        Real stdDeviation(Real strike) const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real M(Real a,Real b,Real rho) const;
        Real d1(Real strike, Rate b)const;
        Real d2(Real strike, Rate b)const;
        Real e1(Real barrier, Real strike, Rate b) const;
        Real e2(Real barrier, Real strike, Rate b) const;
        Real e3(Real barrier, Real strike, Rate b) const;
        Real e4(Real barrier, Real strike, Rate b) const;
        Real f1(Real barrier, Real strike, Rate b) const;
        Real f2(Real barrier, Real strike, Rate b) const;
        Real rho() const;
        Rate mu(Real strike, Rate b) const;
        Real CoB2(PartialBarrier::Type barrierType, Real barrier, Real strike, Rate r, Rate q) const;
        Real CoB1(Real barrier, Real strike, Rate r, Rate q) const;
        Real CA(Integer n, Real barrier, Real strike, Rate r, Rate q) const;
        Real CIA(Integer n, Real barrier, Real strike, Rate r, Rate q) const;
        Real g1(Real barrier, Real strike, Rate b)const;
        Real g2(Real barrier, Real strike, Rate b)const;
        Real g3(Real barrier, Real strike, Rate b)const;
        Real g4(Real barrier, Real strike, Rate b)const;
        Real HS(Real S, Real H, Real power)const;
    };

}


#endif
