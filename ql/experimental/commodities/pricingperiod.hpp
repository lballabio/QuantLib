/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file pricingperiod.hpp
    \brief Pricing period
*/

#ifndef quantlib_pricing_period_hpp
#define quantlib_pricing_period_hpp

#include <ql/experimental/commodities/dateinterval.hpp>
#include <ql/experimental/commodities/quantity.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Time pricingperiod described by a number of a given time unit
    /*! \ingroup datetime */
    class PricingPeriod : public DateInterval {
      public:
        PricingPeriod(const Date& startDate,
                      const Date& endDate,
                      const Date& paymentDate,
                      Quantity quantity)
        : DateInterval(startDate, endDate), paymentDate_(paymentDate),
          quantity_(std::move(quantity)) {}
        const Date& paymentDate() const { return paymentDate_; }
        const Quantity& quantity() const { return quantity_; }
      private:
        Date paymentDate_;
        Quantity quantity_;
    };

    typedef std::vector<ext::shared_ptr<PricingPeriod> > PricingPeriods;
}

#endif
