/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file makeswaption.hpp
    \brief Helper class to instantiate standard market swaption.
*/

#ifndef quantlib_make_swaption_hpp
#define quantlib_make_swaption_hpp

#include <ql/time/businessdayconvention.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    class SwapIndex;
    class Swap;
    class Calendar;
    class IborIndex;
    class Period;
    class PricingEngine;

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market swaption.
    */
    class MakeSwaption {
      public:
        MakeSwaption(ext::shared_ptr<SwapIndex> swapIndex,
                     const Period& optionTenor,
                     Rate strike = Null<Rate>());

        MakeSwaption(ext::shared_ptr<SwapIndex> swapIndex,
                     const Date& fixingDate,
                     Rate strike = Null<Rate>());

        operator Swaption() const;
        operator ext::shared_ptr<Swaption>() const ;

        MakeSwaption& withNominal(Real n);
        MakeSwaption& withSettlementType(Settlement::Type delivery);
        MakeSwaption& withSettlementMethod(Settlement::Method settlementMethod);
        MakeSwaption& withOptionConvention(BusinessDayConvention bdc);
        MakeSwaption& withExerciseDate(const Date&);
        MakeSwaption& withUnderlyingType(Swap::Type type);
        MakeSwaption& withIndexedCoupons(const ext::optional<bool>& b = true);
        MakeSwaption& withAtParCoupons(bool b = true);

        MakeSwaption& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);
      private:
        ext::shared_ptr<SwapIndex> swapIndex_;
        Settlement::Type delivery_;
        Settlement::Method settlementMethod_;
        mutable ext::shared_ptr<FixedVsFloatingSwap> underlyingSwap_;

        Period optionTenor_;
        BusinessDayConvention optionConvention_;
        mutable Date fixingDate_;
        Date exerciseDate_;
        mutable ext::shared_ptr<Exercise> exercise_;

        Rate strike_;
        Swap::Type underlyingType_;
        Real nominal_;
        ext::optional<bool> useIndexedCoupons_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
