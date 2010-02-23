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

#include <ql/experimental/commodities/paymentterm.hpp>

namespace QuantLib {

    std::map<std::string, boost::shared_ptr<PaymentTerm::Data> >
    PaymentTerm::paymentTerms_;

    PaymentTerm::PaymentTerm(const std::string& name,
                             PaymentTerm::EventType eventType,
                             Integer offsetDays,
                             const Calendar& calendar) {
        std::map<std::string, boost::shared_ptr<PaymentTerm::Data> >::const_iterator i = paymentTerms_.find(name);
        if (i != paymentTerms_.end())
            data_ = i->second;
        else {
            data_ = boost::shared_ptr<PaymentTerm::Data>(
                                 new PaymentTerm::Data(name, eventType,
                                                       offsetDays, calendar));
            paymentTerms_[name] = data_;
        }
    }

    std::ostream& operator<<(std::ostream& out, const PaymentTerm& c) {
        if (!c.empty())
            return out << c.name();
        else
            return out << "null payment term type";
    }

}

