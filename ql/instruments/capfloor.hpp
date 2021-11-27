/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2014 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006, 2007 StatPro Italia srl

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

/*! \file capfloor.hpp
    \brief cap and floor class
*/

#ifndef quantlib_instruments_capfloor_hpp
#define quantlib_instruments_capfloor_hpp

#include <ql/instrument.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/handle.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    class YieldTermStructure;

    //! Base class for cap-like instruments
    /*! \ingroup instruments

        \test
        - the correctness of the returned value is tested by checking
          that the price of a cap (resp. floor) decreases
          (resp. increases) with the strike rate.
        - the relationship between the values of caps, floors and the
          resulting collars is checked.
        - the put-call parity between the values of caps, floors and
          swaps is checked.
        - the correctness of the returned implied volatility is tested
          by using it for reproducing the target value.
        - the correctness of the returned value is tested by checking
          it against a known good value.
    */
    class CapFloor : public Instrument {
      public:
        enum Type { Cap, Floor, Collar };
        class arguments;
        class engine;
        CapFloor(Type type,
                 Leg floatingLeg,
                 std::vector<Rate> capRates,
                 std::vector<Rate> floorRates);
        CapFloor(Type type, Leg floatingLeg, const std::vector<Rate>& strikes);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        const std::vector<Rate>& capRates() const { return capRates_; }
        const std::vector<Rate>& floorRates() const { return floorRates_; }
        const Leg& floatingLeg() const { return floatingLeg_; }

        Date startDate() const;
        Date maturityDate() const;
        ext::shared_ptr<FloatingRateCoupon> lastFloatingRateCoupon() const;
        //! Returns the n-th optionlet as a new CapFloor with only one cash flow.
        ext::shared_ptr<CapFloor> optionlet(Size n) const;
        //@}
        Rate atmRate(const YieldTermStructure& discountCurve) const;
        //! implied term volatility
        Volatility impliedVolatility(
                                 Real price,
                                 const Handle<YieldTermStructure>& disc,
                                 Volatility guess,
                                 Real accuracy = 1.0e-4,
                                 Natural maxEvaluations = 100,
                                 Volatility minVol = 1.0e-7,
                                 Volatility maxVol = 4.0,
                                 VolatilityType type = ShiftedLognormal,
                                 Real displacement = 0.0) const;
      private:
        Type type_;
        Leg floatingLeg_;
        std::vector<Rate> capRates_;
        std::vector<Rate> floorRates_;
    };

    //! Concrete cap class
    /*! \ingroup instruments */
    class Cap : public CapFloor {
      public:
        Cap(const Leg& floatingLeg,
            const std::vector<Rate>& exerciseRates)
        : CapFloor(CapFloor::Cap, floatingLeg,
                   exerciseRates, std::vector<Rate>()) {}
    };

    //! Concrete floor class
    /*! \ingroup instruments */
    class Floor : public CapFloor {
      public:
        Floor(const Leg& floatingLeg,
              const std::vector<Rate>& exerciseRates)
        : CapFloor(CapFloor::Floor, floatingLeg,
                   std::vector<Rate>(), exerciseRates) {}
    };

    //! Concrete collar class
    /*! \ingroup instruments */
    class Collar : public CapFloor {
      public:
        Collar(const Leg& floatingLeg,
               const std::vector<Rate>& capRates,
               const std::vector<Rate>& floorRates)
        : CapFloor(CapFloor::Collar, floatingLeg, capRates, floorRates) {}
    };


    //! %Arguments for cap/floor calculation
    class CapFloor::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() : type(CapFloor::Type(-1)) {}
        CapFloor::Type type;
        std::vector<Date> startDates;
        std::vector<Date> fixingDates;
        std::vector<Date> endDates;
        std::vector<Time> accrualTimes;
        std::vector<Rate> capRates;
        std::vector<Rate> floorRates;
        std::vector<Rate> forwards;
        std::vector<Real> gearings;
        std::vector<Real> spreads;
        std::vector<Real> nominals;
        std::vector<ext::shared_ptr<InterestRateIndex> > indexes;
        void validate() const override;
    };

    //! base class for cap/floor engines
    class CapFloor::engine
        : public GenericEngine<CapFloor::arguments, CapFloor::results> {};

    std::ostream& operator<<(std::ostream&, CapFloor::Type);

}

#endif
