/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file commoditypricinghelpers.hpp
    \brief Commodity pricing helpers
*/

#ifndef quantlib_payment_term_hpp
#define quantlib_payment_term_hpp

#include <ql/time/calendar.hpp>
#include <map>

namespace QuantLib {

    class PaymentTerm {
      public:
        enum EventType { TradeDate, PricingDate };

        PaymentTerm();
        PaymentTerm(const std::string& name,
                    EventType eventType,
                    Integer offsetDays,
                    const Calendar& calendar);
        //! \name Inspectors
        //@{
        //! name, e.g, "Pricing end + 5 days"
        const std::string& name() const;
        EventType eventType() const;
        Integer offsetDays() const;
        const Calendar& calendar() const;

        bool empty() const;
        //@}
        Date getPaymentDate(const Date& date) const;
      protected:
        struct Data;
        boost::shared_ptr<Data> data_;

        struct Data {
            std::string name;
            EventType eventType;
            Integer offsetDays;
            Calendar calendar;

            Data(const std::string& name, EventType eventType, Integer offsetDays,
                 const Calendar& calendar);
        };

        static std::map<std::string, boost::shared_ptr<Data> > paymentTerms_;
    };

    /*! \relates PaymentTerm */
    bool operator==(const PaymentTerm&,
                    const PaymentTerm&);

    /*! \relates PaymentTerm */
    bool operator!=(const PaymentTerm&,
                    const PaymentTerm&);

    /*! \relates PaymentTerm */
    std::ostream& operator<<(std::ostream&,
                             const PaymentTerm&);


    inline PaymentTerm::Data::Data(const std::string& name,
                                   PaymentTerm::EventType eventType,
                                   Integer offsetDays,
                                   const Calendar& calendar)
    : name(name), eventType(eventType),
      offsetDays(offsetDays), calendar(calendar) {}

    inline PaymentTerm::PaymentTerm() {}

    inline const std::string& PaymentTerm::name() const {
        return data_->name;
    }

    inline PaymentTerm::EventType PaymentTerm::eventType() const {
        return data_->eventType;
    }

    inline Integer PaymentTerm::offsetDays() const {
        return data_->offsetDays;
    }

    inline Date PaymentTerm::getPaymentDate(const Date& date) const {
        return data_->calendar.adjust(date + data_->offsetDays);
    }

    inline bool PaymentTerm::empty() const {
        return !data_;
    }

    inline bool operator==(const PaymentTerm& c1, const PaymentTerm& c2) {
        return c1.name() == c2.name();
    }

    inline bool operator!=(const PaymentTerm& c1, const PaymentTerm& c2) {
        return !(c1 == c2);
    }

}


#endif
