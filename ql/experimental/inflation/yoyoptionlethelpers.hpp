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

/*! \file yoyoptionlethelpers.hpp
    \brief helpers for YoY inflation-volatility bootstrap
*/

#ifndef quantlib_yoy_optionlet_helpers_hpp
#define quantlib_yoy_optionlet_helpers_hpp

#include <ql/experimental/inflation/yoyinflationoptionletvolatilitystructure2.hpp>
#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    //! Year-on-year inflation-volatility bootstrap helper.
    class YoYOptionletHelper
        : public BootstrapHelper<YoYOptionletVolatilitySurface> {
      public:
        YoYOptionletHelper(const Handle<Quote>& price,
                           Real notional, // get the price level right, e.g., bps = 10,000
                           YoYInflationCapFloor::Type capFloorType,
                           Period& lag,
                           DayCounter yoyDayCounter,
                           Calendar paymentCalendar,
                           Natural fixingDays,
                           ext::shared_ptr<YoYInflationIndex> index,
                           CPI::InterpolationType interpolation,
                           Rate strike,
                           Size n,
                           ext::shared_ptr<YoYInflationCapFloorEngine> pricer);
        /*! \deprecated Use the overload that passes an interpolation type instead.
                        Deprecated in version 1.36.
        */
        [[deprecated("Use the overload that passes an interpolation type instead")]]
        YoYOptionletHelper(const Handle<Quote>& price,
                           Real notional,
                           YoYInflationCapFloor::Type capFloorType,
                           Period& lag,
                           DayCounter yoyDayCounter,
                           Calendar paymentCalendar,
                           Natural fixingDays,
                           ext::shared_ptr<YoYInflationIndex> index,
                           Rate strike,
                           Size n,
                           ext::shared_ptr<YoYInflationCapFloorEngine> pricer);
        void setTermStructure(YoYOptionletVolatilitySurface*) override;
        Real impliedQuote() const override;

      protected:
        Real notional_; // get the price level right (e.g. bps = 10,000)
        YoYInflationCapFloor::Type capFloorType_;
        Period lag_;
        Natural fixingDays_;
        ext::shared_ptr<YoYInflationIndex> index_;
        Rate strike_;
        Size n_;  // how many payments
        DayCounter yoyDayCounter_;
        Calendar calendar_;
        ext::shared_ptr<YoYInflationCapFloorEngine> pricer_;
        // what you make
        ext::shared_ptr<YoYInflationCapFloor> yoyCapFloor_;
    };

}

#endif

