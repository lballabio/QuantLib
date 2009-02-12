/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file inflationcapfloor.hpp
    \brief Inflation cap/floor
*/

#ifndef quantlib_inflation_cap_floor_hpp
#define quantlib_inflation_cap_floor_hpp

#include <ql/experimental/inflation/inflationcoupon.hpp>

namespace QuantLib {

    typedef std::vector<boost::shared_ptr<YoYInflationCoupon> > YoYInflationLeg;

    //! Inflation cap/floor
    /*! This class works as an wrapper to cap/floored inflation coupons.

        Aim is to change code as little as possible but re-write the
        instrument/engine part to make proper use of (inflation)
        coupon pricer.  This makes things MUCH simpler as you just
        pass the coupons and rates to the pricer straight.  I.e. no
        messing about reading all the parameters.  Since the pricer is
        a wrapper on the couponpricer it knows how to do the pricing.

        The limitation is that engines now need to know about
        coupons... but this is specific anyway.

        Note that this version is not generic.  It only works with
        CapFlooredYoYInflation coupons.  Later this can be made more
        general but "premature optimization is the root of many
        errors".
    */
    class YoYInflationCapFloor : public Instrument {
      public:
        enum Type { Cap, Floor, Collar };
        class arguments;
        class engine;
        //! Overrides any previous cap/floor specification on the coupons.
        YoYInflationCapFloor(Type type,
                             const YoYInflationLeg& floatingLeg,
                             const std::vector<Rate>& capRates,
                             const std::vector<Rate>& floorRates);
        YoYInflationCapFloor(Type type,
                             const YoYInflationLeg& floatingLeg,
                             const std::vector<Rate>& strikes);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
        //@}
        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        const YoYInflationLeg& leg() const { return floatingLeg_; }
        const std::vector<Rate>& capRates() const { return capRates_; }
        const std::vector<Rate>& floorRates() const { return floorRates_; }
        std::vector<Date> fixingDates() const;
        std::vector<Date> paymentDates() const;

        Date startDate() const;
        Date maturityDate() const;
        //@}

      private:
        Type type_;
        YoYInflationLeg floatingLeg_;
        std::vector<Rate> capRates_;
        std::vector<Rate> floorRates_;
    };



    //! Inflation cap
    class YoYInflationCap : public YoYInflationCapFloor {
      public:
        YoYInflationCap(const YoYInflationLeg& floatingLeg,
                        const std::vector<Rate>& exerciseRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Cap, floatingLeg,
                               exerciseRates, std::vector<Rate>()) {}
    };


    //! Inflation floor
    class YoYInflationFloor : public YoYInflationCapFloor {
      public:
        YoYInflationFloor(const YoYInflationLeg& floatingLeg,
                          const std::vector<Rate>& exerciseRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Floor, floatingLeg,
                               std::vector<Rate>(), exerciseRates) {}
    };


    //! Inflation collar
    class YoYInflationCollar : public YoYInflationCapFloor {
      public:
        YoYInflationCollar(const YoYInflationLeg& floatingLeg,
                           const std::vector<Rate>& capRates,
                           const std::vector<Rate>& floorRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Collar, floatingLeg,
                               capRates, floorRates) {}
    };


    //! %Arguments for inflation cap/floor calculation
    class YoYInflationCapFloor::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments() : type(YoYInflationCapFloor::Type(-1)) {}
        YoYInflationCapFloor::Type type;
        std::vector<Rate> capRates;
        std::vector<Rate> floorRates;
        YoYInflationLeg floatingLeg;
        void validate() const;
    };

    //! base class for cap/floor engines
    class YoYInflationCapFloor::engine
        : public GenericEngine<YoYInflationCapFloor::arguments,
                               YoYInflationCapFloor::results> {};

    std::ostream& operator<<(std::ostream&, YoYInflationCapFloor::Type);


    //! helper class for YoY Inflation caps or floors (or -let versions)
    class MakeYoYInflationCapFloor {
      public:
        MakeYoYInflationCapFloor(YoYInflationCapFloor::Type capFloorType,
                                 Period &lag, Natural fixingDays,
                                 const boost::shared_ptr<YoYInflationIndex>&,
                                 Rate strike, Size n);

        operator YoYInflationCapFloor() const;
        operator boost::shared_ptr<YoYInflationCapFloor>() const ;

        MakeYoYInflationCapFloor& withNominal(Real nominal);
        MakeYoYInflationCapFloor& withTenor(Size n);
        MakeYoYInflationCapFloor& withEffectiveDate(const Date& effectiveDate);

        MakeYoYInflationCapFloor& withCalendar(const Calendar& cal);
        MakeYoYInflationCapFloor& withConvention(BusinessDayConvention bdc);
        MakeYoYInflationCapFloor& withDayCount(const DayCounter& dc);

        //! this makes only the last floor-let or cap-let
        MakeYoYInflationCapFloor& asOptionlet();

        // N.B. inflation indices contain yield curves that are used
        // for discounting
        MakeYoYInflationCapFloor& withPricingEngine(
                              const boost::shared_ptr<PricingEngine>& pricer);

      private:
        YoYInflationCapFloor::Type capFloorType_;
        bool asOptionlet_;
        Period lag_;
        Natural fixingDays_;
        boost::shared_ptr<YoYInflationIndex> index_;
        Rate strike_;
        Size tenorN_;
        Real nominal_;
        Date effectiveDate_;
        bool firstCapletExcluded_;
        Calendar cal_;
        BusinessDayConvention bdc_;
        DayCounter dc_;
        boost::shared_ptr<PricingEngine> pricer_;
    };

}

#endif

