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
 <http://quantlib.org/license.shtml>.

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
        MakeSwaption(const boost::shared_ptr<SwapIndex>& swapIndex,
                     const Period& optionTenor,
                     Rate strike = Null<Rate>());

        MakeSwaption(const boost::shared_ptr<SwapIndex>& swapIndex,
                     const Date& fixingDate,
                     Rate strike = Null<Rate>());

        operator Swaption() const;
        operator boost::shared_ptr<Swaption>() const ;

        MakeSwaption& withSettlementType(Settlement::Type delivery);
        MakeSwaption& withOptionConvention(BusinessDayConvention bdc);
        MakeSwaption& withExerciseDate(const Date&);
        MakeSwaption& withUnderlyingType(const VanillaSwap::Type type);

        MakeSwaption& withPricingEngine(
                              const boost::shared_ptr<PricingEngine>& engine);
      private:
        boost::shared_ptr<SwapIndex> swapIndex_;
        Settlement::Type delivery_;
        mutable boost::shared_ptr<VanillaSwap> underlyingSwap_;

        Period optionTenor_;
        BusinessDayConvention optionConvention_;
        mutable Date fixingDate_;
        Date exerciseDate_;
        mutable boost::shared_ptr<Exercise> exercise_;

        Rate strike_;
        VanillaSwap::Type underlyingType_;

        boost::shared_ptr<PricingEngine> engine_;
    };

}

#endif
