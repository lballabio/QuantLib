/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

/*! \file makeois.hpp
    \brief Helper class to instantiate overnight indexed swaps.
*/

#ifndef quantlib_makearithmeticaverageois_hpp
#define quantlib_makearithmeticaverageois_hpp

#include <ql/experimental/averageois/arithmeticaverageois.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate arithemtic average overnight indexed swaps.
    */
    class MakeArithmeticAverageOIS {
      public:
        MakeArithmeticAverageOIS(const Period& swapTenor,
                const ext::shared_ptr<OvernightIndex>& overnightIndex,
                Rate fixedRate = Null<Rate>(),
                const Period& fwdStart = 0*Days);

        operator ArithmeticAverageOIS() const;
        operator ext::shared_ptr<ArithmeticAverageOIS>() const;

        MakeArithmeticAverageOIS& receiveFixed(bool flag = true);
        MakeArithmeticAverageOIS& withType(Swap::Type type);
        MakeArithmeticAverageOIS& withNominal(Real n);

        MakeArithmeticAverageOIS& withSettlementDays(Natural settlementDays);
        MakeArithmeticAverageOIS& withEffectiveDate(const Date&);
        MakeArithmeticAverageOIS& withTerminationDate(const Date&);
        MakeArithmeticAverageOIS& withRule(DateGeneration::Rule r);

        MakeArithmeticAverageOIS& withFixedLegPaymentFrequency(Frequency f);
        MakeArithmeticAverageOIS& withOvernightLegPaymentFrequency(Frequency f);
        MakeArithmeticAverageOIS& withEndOfMonth(bool flag = true);

        MakeArithmeticAverageOIS& withFixedLegDayCount(const DayCounter& dc);

        MakeArithmeticAverageOIS& withOvernightLegSpread(Spread sp);

        MakeArithmeticAverageOIS& withDiscountingTermStructure(
                  const Handle<YieldTermStructure>& discountingTermStructure);
        MakeArithmeticAverageOIS& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);
        MakeArithmeticAverageOIS& withArithmeticAverage(
                                       Real meanReversionSpeed = 0.03,
                                       Real volatility = 0.00, // NO convexity adjustment by default
                                       bool byApprox = false); // TRUE to use Katsumi Takada approximation
      private:
        Period swapTenor_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_ = 2;
        Date effectiveDate_, terminationDate_;
        Calendar calendar_;

        Frequency fixedLegPaymentFrequency_ = Annual;
        Frequency overnightLegPaymentFrequency_ = Annual;
        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_, isDefaultEOM_ = true;

        bool byApprox_ = false;
        Real mrs_ = 0.03;
        Real vol_ = 0.00;

        Swap::Type type_ = Swap::Payer;
        Real nominal_ = 1.0;

        Spread overnightSpread_ = 0.0;
        DayCounter fixedDayCount_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
