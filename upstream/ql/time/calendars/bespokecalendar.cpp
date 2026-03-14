/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/time/calendars/bespokecalendar.hpp>
#include <sstream>
#include <utility>

namespace QuantLib {

    BespokeCalendar::Impl::Impl(std::string name) : name_(std::move(name)) {}

    std::string BespokeCalendar::Impl::name() const {
        return name_;
    }

    bool BespokeCalendar::Impl::isWeekend(Weekday w) const {
        return (weekend_mask_ & (1 << w)) != 0;
    }

    bool BespokeCalendar::Impl::isBusinessDay(const Date& date) const {
        return !isWeekend(date.weekday());
    }

    void BespokeCalendar::Impl::addWeekend(Weekday w) {
        weekend_mask_ |= (1 << w);
    }


    BespokeCalendar::BespokeCalendar(const std::string& name) {
        bespokeImpl_ = ext::make_shared<BespokeCalendar::Impl>(
                                             name);
        impl_ = bespokeImpl_;
    }

    void BespokeCalendar::addWeekend(Weekday w) {
        bespokeImpl_->addWeekend(w);
    }

}

