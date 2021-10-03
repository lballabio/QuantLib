/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file makecapfloor.hpp
    \brief Helper class to instantiate standard market cap/floor.
*/

#ifndef quantlib_instruments_makecapfloor_hpp
#define quantlib_instruments_makecapfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/makevanillaswap.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market cap and floor.
    */
    class MakeCapFloor {
      public:
        MakeCapFloor(CapFloor::Type capFloorType,
                     const Period& capFloorTenor,
                     const ext::shared_ptr<IborIndex>& iborIndex,
                     Rate strike = Null<Rate>(),
                     const Period& forwardStart = 0*Days);

        operator CapFloor() const;
        operator ext::shared_ptr<CapFloor>() const;

        MakeCapFloor& withNominal(Real n);
        MakeCapFloor& withEffectiveDate(const Date& effectiveDate,
                                        bool firstCapletExcluded);
        MakeCapFloor& withTenor(const Period& t);
        MakeCapFloor& withCalendar(const Calendar& cal);
        MakeCapFloor& withConvention(BusinessDayConvention bdc);
        MakeCapFloor& withTerminationDateConvention(BusinessDayConvention bdc);
        MakeCapFloor& withRule(DateGeneration::Rule r);
        MakeCapFloor& withEndOfMonth(bool flag = true);
        MakeCapFloor& withFirstDate(const Date& d);
        MakeCapFloor& withNextToLastDate(const Date& d);
        MakeCapFloor& withDayCount(const DayCounter& dc);
        MakeCapFloor& withSettlementDays(Natural SettlementDates);

        //! only get last coupon
        MakeCapFloor& asOptionlet(bool b = true);

        MakeCapFloor& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);
      private:
        CapFloor::Type capFloorType_;
        Rate strike_;
        bool firstCapletExcluded_, asOptionlet_;

        MakeVanillaSwap makeVanillaSwap_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
