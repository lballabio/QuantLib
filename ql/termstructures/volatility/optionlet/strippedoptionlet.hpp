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

/*! \file optionletstripper.hpp
*/

#ifndef quantlib_strippedoptionlet_hpp
#define quantlib_strippedoptionlet_hpp

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
//#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>

namespace QuantLib {

    class IborIndex;

    class StrippedOptionlet : public StrippedOptionletBase {
      public:
        StrippedOptionlet(const Date& referenceDate,
                          const Calendar& calendar,
                          Natural settlementDays,
                          BusinessDayConvention businessDayConvention,
                          const DayCounter& dc,
                          const boost::shared_ptr<IborIndex>& index,
                          const std::vector<Period>& optionletTenors,
                          const std::vector<Rate>& strikes,
                          const std::vector<std::vector<Handle<Quote> > >&
                          );

        //! \name StrippedOptionletBase interface
        //@{
        const std::vector<Rate>& optionletStrikes(Size i) const;
        const std::vector<Volatility>& optionletVolatilities(Size i) const;

        const std::vector<Date>& optionletDates() const;
        const std::vector<Time>& optionletTimes() const;

        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        BusinessDayConvention businessDayConvention() const;
        const Date& referenceDate() const;
        //@}

        const std::vector<Period>& optionletTenors() const;

      protected:
        
        void checkInputs() const;
        void registerWithMarketData();

        const Date referenceDate_;
        const Calendar calendar_;
        Natural settlementDays_;
        BusinessDayConvention businessDayConvention_;
        const DayCounter dc_;
        const boost::shared_ptr<IborIndex> index_;

        Size nStrikes_; 
        Size nOptionletTenors_;

        mutable std::vector<std::vector<Rate> > optionletStrikes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;

        mutable std::vector<Time> optionletTimes_;
        mutable std::vector<Date> optionletDates_;
        std::vector<Period> optionletTenors_;

      private:
        void performCalculations() const;
        const std::vector<std::vector<Handle<Quote> > > optionletVolQuotes_;

    };

}

#endif
