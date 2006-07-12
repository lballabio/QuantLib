/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */
/*! \file swaprate.hpp
    \brief swap-rate indexes
*/

#ifndef quantlib_swap_rate_hpp
#define quantlib_swap_rate_hpp

#include <ql/qldefines.hpp>
#include <ql/Indexes/core.hpp>

namespace QuantLib {

    //! base class for swap-rate indexes
    class SwapRate : public Index, public Observer {
      public:
        SwapRate(const std::string& familyName,
                 Integer years,
                 Integer settlementDays,
                 Currency currency,
                 const Calendar& calendar,
                 Frequency fixedLegFrequency,
                 BusinessDayConvention fixedLegConvention,
                 const DayCounter& fixedLegDayCounter,
                 const boost::shared_ptr<Xibor>& index,
                 Integer indexFixingDays,
                 Frequency floatingLegFrequency,
                 BusinessDayConvention floatingLegConvention)
        : familyName_(familyName), years_(years),
          settlementDays_(settlementDays),
          currency_(currency), calendar_(calendar),
          index_(index), indexFixingDays_(indexFixingDays),
          fixedLegFrequency_(fixedLegFrequency),
          floatingLegFrequency_(floatingLegFrequency),
          fixedLegConvention_(fixedLegConvention),
          floatingLegConvention_(floatingLegConvention),
          fixedLegDayCounter_(fixedLegDayCounter) {
            registerWith(index_);
        }
        //! \name Index interface
        //@{
        std::string name() const;
        Rate fixing(const Date& fixingDate) const;
        //@}
        //! \name Inspectors
        //@{
        const Calendar& calendar() const { return calendar_; }
        const Currency& currency() const { return currency_; }
        Period tenor() const { return Period(years_, Years); }
        Frequency fixedLegFrequency() const {
            return fixedLegFrequency_;
        }
        BusinessDayConvention fixedLegConvention() const {
            return fixedLegConvention_;
        }
        const DayCounter fixedLegDayCounter() const {
            return fixedLegDayCounter_;
        }
        Frequency floatingLegFrequency() const {
            return floatingLegFrequency_;
        }
        BusinessDayConvention floatingLegConvention() const {
            return floatingLegConvention_;
        }
        boost::shared_ptr<Xibor> libor() const { return index_; }
        Integer indexFixingDays() const { return indexFixingDays_; }
        boost::shared_ptr<YieldTermStructure> termStructure() const {
            return index_->termStructure();
        }
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
      private:
        std::string familyName_;
        Integer years_;
        Integer settlementDays_;
        Currency currency_;
        Calendar calendar_;
        boost::shared_ptr<Xibor> index_;
        Integer indexFixingDays_;
        Frequency fixedLegFrequency_, floatingLegFrequency_;
        BusinessDayConvention fixedLegConvention_, floatingLegConvention_;
        DayCounter fixedLegDayCounter_;
    };

}


#endif
