/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2005 Theo Boafo

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dividendschedule.hpp
    \brief Schedule of dividend dates
*/

#ifndef quantlib_dividend_schedule_hpp
#define quantlib_dividend_schedule_hpp

#include <ql/date.hpp>
#include <ql/cashflow.hpp>
#include <vector>

namespace QuantLib {

    class DividendSchedule {
      public:
        std::vector<boost::shared_ptr<CashFlow> > cashFlow;
        std::vector<boost::shared_ptr<Event > > getEventList() const {
            std::vector<boost::shared_ptr<Event > > event_list;
            std::copy(cashFlow.begin(), cashFlow.end(),
                      std::back_inserter(event_list));
            return event_list;
        }
    };

}


#endif

