/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file makeswaptions.hpp
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
                     Rate strike = Null<Rate>(),
                     const Period& forwardStart = 0*Days,
                     const boost::shared_ptr<PricingEngine>& engine =
                         boost::shared_ptr<PricingEngine>());

        operator Swaption() const;
        operator boost::shared_ptr<Swaption>() const ;

        MakeSwaption& withSwaptionConvention(BusinessDayConvention bdc);
        MakeSwaption& withSettlementType(Settlement::Type delivery);

      private:
        void create() const;

        Settlement::Type delivery_;
        mutable Rate strike_;
        Period optionTenor_;
        boost::shared_ptr<SwapIndex> swapIndex_;
        BusinessDayConvention swaptionConvention_;

        boost::shared_ptr<PricingEngine> engine_;
        mutable boost::shared_ptr<Exercise> exercise_;
        mutable boost::shared_ptr<VanillaSwap> underlyingSwap_;
    };

}

#endif
