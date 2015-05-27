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

/*! \file optionletstripper.hpp
    \brief optionlet (caplet/floorlet) volatility stripper
*/

#ifndef quantlib_optionletstripper_hpp
#define quantlib_optionletstripper_hpp

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class IborIndex;

    /*! StrippedOptionletBase specialization. It's up to derived
        classes to implement LazyObject::performCalculations
    */
    class OptionletStripper : public StrippedOptionletBase {
      public:
        //! \name StrippedOptionletBase interface
        //@{
        const std::vector<Rate>& optionletStrikes(Size i) const;
        const std::vector<Volatility>& optionletVolatilities(Size i) const;

        const std::vector<Date>& optionletFixingDates() const;
        const std::vector<Time>& optionletFixingTimes() const;
        Size optionletMaturities() const;

        const std::vector<Rate>& atmOptionletRates() const;

        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        BusinessDayConvention businessDayConvention() const;
        //@}

        const std::vector<Period>& optionletFixingTenors() const;
        const std::vector<Date>& optionletPaymentDates() const;
        const std::vector<Time>& optionletAccrualPeriods() const;
        boost::shared_ptr<CapFloorTermVolSurface> termVolSurface() const;
        boost::shared_ptr<IborIndex> iborIndex() const;
        Real displacement() const;
        VolatilityType volatilityType() const;

      protected:
        OptionletStripper(const boost::shared_ptr<CapFloorTermVolSurface>&,
                          const boost::shared_ptr<IborIndex>& iborIndex_,
                          const Handle<YieldTermStructure>& discount =
                                                 Handle<YieldTermStructure>(),
                          VolatilityType type = ShiftedLognormal,
                          Real displacement = 0.0);
        boost::shared_ptr<CapFloorTermVolSurface> termVolSurface_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Handle<YieldTermStructure> discount_;
        Size nStrikes_;
        Size nOptionletTenors_;

        mutable std::vector<std::vector<Rate> > optionletStrikes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;

        mutable std::vector<Time> optionletTimes_;
        mutable std::vector<Date> optionletDates_;
        std::vector<Period> optionletTenors_;
        mutable std::vector<Rate> atmOptionletRate_;
        mutable std::vector<Date> optionletPaymentDates_;
        mutable std::vector<Time> optionletAccrualPeriods_;

        std::vector<Period> capFloorLengths_;
        VolatilityType volatilityType_;
        Real displacement_;
    };

}

#endif
