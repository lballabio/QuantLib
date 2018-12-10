/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file floatfloatswap.hpp
    \brief swap exchanging capped floored Libor or CMS coupons with quite
           general specification. If no payment convention is given, the
           respective leg schedule convention is used. The interest rate
           indices should be linked to valid forwarding and in case of
           swap indices discounting curves
*/

#ifndef quantlib_floatfloat_swap_hpp
#define quantlib_floatfloat_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class InterestRateIndex;

    //! float float swap

    class FloatFloatSwap : public Swap {
      public:
        class arguments;
        class results;
        class engine;
        FloatFloatSwap(
            const VanillaSwap::Type type, const Real nominal1,
            const Real nominal2, const Schedule &schedule1,
            const ext::shared_ptr<InterestRateIndex> &index1,
            const DayCounter &dayCount1, const Schedule &schedule2,
            const ext::shared_ptr<InterestRateIndex> &index2,
            const DayCounter &dayCount2,
            const bool intermediateCapitalExchange = false,
            const bool finalCapitalExchange = false, const Real gearing1 = 1.0,
            const Real spread1 = 0.0, const Real cappedRate1 = Null<Real>(),
            const Real flooredRate1 = Null<Real>(), const Real gearing2 = 1.0,
            const Real spread2 = 0.0, const Real cappedRate2 = Null<Real>(),
            const Real flooredRate2 = Null<Real>(),
            boost::optional<BusinessDayConvention> paymentConvention1 =
                boost::none,
            boost::optional<BusinessDayConvention> paymentConvention2 =
                boost::none);

        FloatFloatSwap(
            const VanillaSwap::Type type, const std::vector<Real> &nominal1,
            const std::vector<Real> &nominal2, const Schedule &schedule1,
            const ext::shared_ptr<InterestRateIndex> &index1,
            const DayCounter &dayCount1, const Schedule &schedule2,
            const ext::shared_ptr<InterestRateIndex> &index2,
            const DayCounter &dayCount2,
            const bool intermediateCapitalExchange = false,
            const bool finalCapitalExchange = false,
            const std::vector<Real> &gearing1 = std::vector<Real>(),
            const std::vector<Real> &spread1 = std::vector<Real>(),
            const std::vector<Real> &cappedRate1 = std::vector<Real>(),
            const std::vector<Real> &flooredRate1 = std::vector<Real>(),
            const std::vector<Real> &gearing2 = std::vector<Real>(),
            const std::vector<Real> &spread2 = std::vector<Real>(),
            const std::vector<Real> &cappedRate2 = std::vector<Real>(),
            const std::vector<Real> &flooredRate2 = std::vector<Real>(),
            boost::optional<BusinessDayConvention> paymentConvention1 =
                boost::none,
            boost::optional<BusinessDayConvention> paymentConvention2 =
                boost::none);

        //! \name Inspectors
        //@{
        VanillaSwap::Type type() const;
        const std::vector<Real> &nominal1() const;
        const std::vector<Real> &nominal2() const;

        const Schedule &schedule1() const;
        const Schedule &schedule2() const;

        const ext::shared_ptr<InterestRateIndex> &index1() const;
        const ext::shared_ptr<InterestRateIndex> &index2() const;

        const std::vector<Real> spread1() const;
        const std::vector<Real> spread2() const;

        const std::vector<Real> gearing1() const;
        const std::vector<Real> gearing2() const;

        const std::vector<Rate> cappedRate1() const;
        const std::vector<Rate> flooredRate1() const;
        const std::vector<Rate> cappedRate2() const;
        const std::vector<Rate> flooredRate2() const;

        const DayCounter &dayCount1() const;
        const DayCounter &dayCount2() const;

        BusinessDayConvention paymentConvention1() const;
        BusinessDayConvention paymentConvention2() const;

        const Leg &leg1() const;
        const Leg &leg2() const;
        //@}

        //! \name Results
        //@{
        //@}
        // other
        void setupArguments(PricingEngine::arguments *args) const;
        void fetchResults(const PricingEngine::results *) const;

      private:
        void init(boost::optional<BusinessDayConvention> paymentConvention1,
                  boost::optional<BusinessDayConvention> paymentConvention2);
        void setupExpired() const;
        VanillaSwap::Type type_;
        std::vector<Real> nominal1_, nominal2_;
        Schedule schedule1_, schedule2_;
        ext::shared_ptr<InterestRateIndex> index1_, index2_;
        std::vector<Real> gearing1_, gearing2_, spread1_, spread2_;
        std::vector<Real> cappedRate1_, flooredRate1_, cappedRate2_,
            flooredRate2_;
        DayCounter dayCount1_, dayCount2_;
        std::vector<bool> isRedemptionFlow1_, isRedemptionFlow2_;
        BusinessDayConvention paymentConvention1_, paymentConvention2_;
        const bool intermediateCapitalExchange_, finalCapitalExchange_;
    };

    //! %Arguments for float float swap calculation
    class FloatFloatSwap::arguments : public Swap::arguments {
      public:
        arguments() : type(VanillaSwap::Receiver) {}
        VanillaSwap::Type type;
        std::vector<Real> nominal1, nominal2;

        std::vector<Date> leg1ResetDates, leg1FixingDates, leg1PayDates;
        std::vector<Date> leg2ResetDates, leg2FixingDates, leg2PayDates;

        std::vector<Real> leg1Spreads, leg2Spreads, leg1Gearings, leg2Gearings;
        std::vector<Real> leg1CappedRates, leg1FlooredRates, leg2CappedRates,
            leg2FlooredRates;

        std::vector<Real> leg1Coupons, leg2Coupons;
        std::vector<Real> leg1AccrualTimes, leg2AccrualTimes;

        ext::shared_ptr<InterestRateIndex> index1, index2;

        std::vector<bool> leg1IsRedemptionFlow, leg2IsRedemptionFlow;

        void validate() const;
    };

    //! %Results from float float swap calculation
    class FloatFloatSwap::results : public Swap::results {
      public:
        void reset();
    };

    class FloatFloatSwap::engine
        : public GenericEngine<FloatFloatSwap::arguments,
                               FloatFloatSwap::results> {};

    // inline definitions

    inline VanillaSwap::Type FloatFloatSwap::type() const { return type_; }

    inline const std::vector<Real> &FloatFloatSwap::nominal1() const {
        return nominal1_;
    }

    inline const std::vector<Real> &FloatFloatSwap::nominal2() const {
        return nominal2_;
    }

    inline const Schedule &FloatFloatSwap::schedule1() const {
        return schedule1_;
    }

    inline const Schedule &FloatFloatSwap::schedule2() const {
        return schedule2_;
    }

    inline const ext::shared_ptr<InterestRateIndex> &
    FloatFloatSwap::index1() const {
        return index1_;
    }

    inline const ext::shared_ptr<InterestRateIndex> &
    FloatFloatSwap::index2() const {
        return index2_;
    }

    inline const std::vector<Real> FloatFloatSwap::spread1() const {
        return spread1_;
    }

    inline const std::vector<Real> FloatFloatSwap::spread2() const {
        return spread2_;
    }

    inline const std::vector<Real> FloatFloatSwap::gearing1() const {
        return gearing1_;
    }

    inline const std::vector<Real> FloatFloatSwap::gearing2() const {
        return gearing2_;
    }

    inline const std::vector<Real> FloatFloatSwap::cappedRate1() const {
        return cappedRate1_;
    }

    inline const std::vector<Real> FloatFloatSwap::cappedRate2() const {
        return cappedRate2_;
    }

    inline const std::vector<Real> FloatFloatSwap::flooredRate1() const {
        return flooredRate1_;
    }

    inline const std::vector<Real> FloatFloatSwap::flooredRate2() const {
        return flooredRate2_;
    }

    inline const DayCounter &FloatFloatSwap::dayCount1() const {
        return dayCount1_;
    }

    inline const DayCounter &FloatFloatSwap::dayCount2() const {
        return dayCount2_;
    }

    inline BusinessDayConvention FloatFloatSwap::paymentConvention1() const {
        return paymentConvention1_;
    }

    inline BusinessDayConvention FloatFloatSwap::paymentConvention2() const {
        return paymentConvention2_;
    }

    inline const Leg &FloatFloatSwap::leg1() const { return legs_[0]; }

    inline const Leg &FloatFloatSwap::leg2() const { return legs_[1]; }
}

#endif
