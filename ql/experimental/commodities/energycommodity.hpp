/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file energycommodity.hpp
    \brief Energy commodity
*/

#ifndef quantlib_energy_commodity_hpp
#define quantlib_energy_commodity_hpp

#include <ql/experimental/commodities/commodity.hpp>
#include <ql/experimental/commodities/commoditytype.hpp>
#include <ql/experimental/commodities/commodityunitcost.hpp>
#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <ql/experimental/commodities/quantity.hpp>
#include <ql/time/date.hpp>
#include <ql/money.hpp>

namespace QuantLib {

    struct EnergyDailyPosition {
        Date date;
        Real quantityAmount;
        Real payLegPrice = 0;
        Real receiveLegPrice = 0;
        Real riskDelta;
        bool unrealized = false;

        EnergyDailyPosition() = default;
        EnergyDailyPosition(const Date& date,
                            Real payLegPrice,
                            Real receiveLegPrice,
                            bool unrealized);
    };

    typedef std::map<Date, EnergyDailyPosition> EnergyDailyPositions;

    #ifndef __DOXYGEN__
    std::ostream& operator<<(std::ostream& out,
                             const EnergyDailyPositions& dailyPositions);
    #endif



    //! Energy commodity class
    /*! \ingroup instruments */
    class EnergyCommodity : public Commodity {
      public:
        class arguments;
        class results;
        class engine;

        enum DeliverySchedule { Constant,
                                Window,
                                Hourly,
                                Daily,
                                Weekly,
                                Monthly,
                                Quarterly,
                                Yearly };
        enum QuantityPeriodicity { Absolute,
                                   PerHour,
                                   PerDay,
                                   PerWeek,
                                   PerMonth,
                                   PerQuarter,
                                   PerYear };
        enum PaymentSchedule { WindowSettlement,
                               MonthlySettlement,
                               QuarterlySettlement,
                               YearlySettlement };

        EnergyCommodity(CommodityType commodityType,
                        const ext::shared_ptr<SecondaryCosts>& secondaryCosts);

        virtual Quantity quantity() const = 0;
        const CommodityType& commodityType() const;

        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;

      protected:
        static Real calculateFxConversionFactor(const Currency& fromCurrency,
                                                const Currency& toCurrency,
                                                const Date& evaluationDate);
        static Real calculateUomConversionFactor(
                                       const CommodityType& commodityType,
                                       const UnitOfMeasure& fromUnitOfMeasure,
                                       const UnitOfMeasure& toUnitOfMeasure);
        Real calculateUnitCost(const CommodityType& commodityType,
                               const CommodityUnitCost& unitCost,
                               const Date& evaluationDate) const;
        void calculateSecondaryCostAmounts(const CommodityType& commodityType,
                                           Real totalQuantityValue,
                                           const Date& evaluationDate) const;

        CommodityType commodityType_;
    };


    class EnergyCommodity::arguments : public virtual PricingEngine::arguments {
      public:
        Currency currency;
        UnitOfMeasure unitOfMeasure;
        void validate() const override {}
    };

    class EnergyCommodity::results : public Instrument::results {
      public:
        Real NPV;
        Currency currency;
        UnitOfMeasure unitOfMeasure;
        void reset() override { Instrument::results::reset(); }
    };

    class EnergyCommodity::engine
        : public GenericEngine<EnergyCommodity::arguments,
                               EnergyCommodity::results> {};

}

#endif


#ifndef id_a39939fe0b2873a33ed7c875327ce224
#define id_a39939fe0b2873a33ed7c875327ce224
inline bool test_a39939fe0b2873a33ed7c875327ce224(int* i) { return i != 0; }
#endif
