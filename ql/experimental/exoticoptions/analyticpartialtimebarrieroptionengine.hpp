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

    template<Option::Type OptionType = Option::Call>
    class AnalyticPartialTimeBarrierOptionEngine
        : public PartialTimeBarrierOption::engine {
      public:
        explicit AnalyticPartialTimeBarrierOptionEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Real calculate(PartialTimeBarrierOption::arguments& arguments) const;
        Real underlying() const;
        Real strike() const;
        Time residualTime() const;
        Time coverEventTime() const;
        Volatility volatility(Time t) const;
        Real barrier() const;
        Real rebate() const;
        Real stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        Real M(Real a,Real b,Real rho) const;
        Real d1()const;
        Real d2()const;
        Real e1() const;
        Real e2() const;
        Real e3() const;
        Real e4() const;
        Real f1() const;
        Real f2() const;
        Real rho() const;
        Rate mu() const;
        Real CoB2(PartialBarrier::Type barrierType) const;
        Real CoB1() const;
        Real CA(Integer n) const;
        Real CIA(Integer n) const;
        Real g1()const;
        Real g2()const;
        Real g3()const;
        Real g4()const;
        Real HS(Real S, Real H, Real power)const;
    };

}


#endif
