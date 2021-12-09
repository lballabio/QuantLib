/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl
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

/*! \file makecapfloor.hpp
 \brief Helper class to instantiate standard yoy inflation cap/floor.
 */

#ifndef quantlib_instruments_make_yoyinflation_capfloor_hpp
#define quantlib_instruments_make_yoyinflation_capfloor_hpp

#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/instruments/makevanillaswap.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way to instantiate
        standard yoy inflation cap and floor.
     */
    class MakeYoYInflationCapFloor {
      public:
        MakeYoYInflationCapFloor(YoYInflationCapFloor::Type capFloorType,
                                 ext::shared_ptr<YoYInflationIndex> index,
                                 const Size& length,
                                 Calendar cal,
                                 const Period& observationLag);
        MakeYoYInflationCapFloor& withNominal(Real n);
        MakeYoYInflationCapFloor& withEffectiveDate(const Date& effectiveDate);
        MakeYoYInflationCapFloor& withFirstCapletExcluded();
        MakeYoYInflationCapFloor& withPaymentDayCounter(const DayCounter&);
        MakeYoYInflationCapFloor& withPaymentAdjustment(BusinessDayConvention);
        MakeYoYInflationCapFloor& withFixingDays(Natural fixingDays);
        MakeYoYInflationCapFloor& withPricingEngine(
                const ext::shared_ptr<PricingEngine>& engine);
        //! only get last coupon
        MakeYoYInflationCapFloor& asOptionlet(bool b = true);
        MakeYoYInflationCapFloor& withStrike(Rate strike);
        MakeYoYInflationCapFloor& withAtmStrike(
                      const Handle<YieldTermStructure>& nominalTermStructure);
        MakeYoYInflationCapFloor& withForwardStart(Period forwardStart);

        operator YoYInflationCapFloor() const;
        operator ext::shared_ptr<YoYInflationCapFloor>() const ;

      private:
        YoYInflationCapFloor::Type capFloorType_;
        Size length_;
        Calendar calendar_;
        ext::shared_ptr<YoYInflationIndex> index_;
        Period observationLag_;
        Rate strike_;
        bool firstCapletExcluded_, asOptionlet_;
        Date effectiveDate_;
        Period forwardStart_;
        DayCounter dayCounter_;
        BusinessDayConvention roll_;
        Natural fixingDays_;
        Real nominal_;
        Handle<YieldTermStructure> nominalTermStructure_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
