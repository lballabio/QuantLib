/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2015 Michael von den Driesch

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

/*! \file strippedoptionlet.hpp
*/

#ifndef quantlib_strippedoptionlet_hpp
#define quantlib_strippedoptionlet_hpp

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    /*! Helper class to wrap in a StrippedOptionletBase object a matrix of
        exogenously calculated optionlet (i.e. caplet/floorlet) volatilities
        (a.k.a. forward-forward volatilities).
    */
    class StrippedOptionlet : public StrippedOptionletBase {
      public:
        StrippedOptionlet(Natural settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention bdc,
                          ext::shared_ptr<IborIndex> iborIndex,
                          const std::vector<Date>& optionletDates,
                          const std::vector<Rate>& strikes,
                          std::vector<std::vector<Handle<Quote> > >,
                          DayCounter dc,
                          VolatilityType type = ShiftedLognormal,
                          Real displacement = 0.0);
        StrippedOptionlet(Natural settlementDays,
                          const Calendar& calendar,
                          BusinessDayConvention bdc,
                          ext::shared_ptr<IborIndex> iborIndex,
                          const std::vector<Date>& optionletDates,
                          const std::vector<std::vector<Rate>>& strikes,
                          std::vector<std::vector<Handle<Quote>>>,
                          DayCounter dc,
                          VolatilityType type = ShiftedLognormal,
                          Real displacement = 0.0);
        //! \name StrippedOptionletBase interface
        //@{
        const std::vector<Rate>& optionletStrikes(Size i) const override;
        const std::vector<Volatility>& optionletVolatilities(Size i) const override;

        const std::vector<Date>& optionletFixingDates() const override;
        const std::vector<Time>& optionletFixingTimes() const override;
        Size optionletMaturities() const override;

        const std::vector<Rate>& atmOptionletRates() const override;

        DayCounter dayCounter() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        BusinessDayConvention businessDayConvention() const override;
        //@}
        VolatilityType volatilityType() const override;
        Real displacement() const override;

      private:
        void checkInputs() const;
        void registerWithMarketData();
        void performCalculations() const override;

        Calendar calendar_;
        Natural settlementDays_;
        BusinessDayConvention businessDayConvention_;
        DayCounter dc_;
        ext::shared_ptr<IborIndex> iborIndex_;
        VolatilityType type_;
        Real displacement_;

        Size nOptionletDates_;
        std::vector<Date> optionletDates_;
        std::vector<Time> optionletTimes_;
        mutable std::vector<Rate> optionletAtmRates_;
        std::vector<std::vector<Rate> > optionletStrikes_;

        std::vector<std::vector<Handle<Quote> > > optionletVolQuotes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;
    };

}

#endif
