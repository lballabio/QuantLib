/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/experimental/coupons/swapspreadindex.hpp>
#include <iomanip>
#include <sstream>
#include <utility>

namespace QuantLib {

    SwapSpreadIndex::SwapSpreadIndex(const std::string& familyName,
                                     const ext::shared_ptr<SwapIndex>& swapIndex1,
                                     ext::shared_ptr<SwapIndex> swapIndex2,
                                     const Real gearing1,
                                     const Real gearing2)
    : InterestRateIndex(familyName,
                        swapIndex1->tenor(), // does not make sense, but we have to provide one
                        swapIndex1->fixingDays(),
                        swapIndex1->currency(),
                        swapIndex1->fixingCalendar(),
                        swapIndex1->dayCounter()),
      swapIndex1_(swapIndex1), swapIndex2_(std::move(swapIndex2)), gearing1_(gearing1),
      gearing2_(gearing2) {

        registerWith(swapIndex1_);
        registerWith(swapIndex2_);

        std::ostringstream name;
        name << std::setprecision(4) << std::fixed << swapIndex1_->name() << "("
             << gearing1 << ") + " << swapIndex2_->name() << "(" << gearing2
             << ")";
        name_ = name.str();

        QL_REQUIRE(swapIndex1_->fixingDays() == swapIndex2_->fixingDays(),
                   "index1 fixing days ("
                       << swapIndex1_->fixingDays() << ")"
                       << "must be equal to index2 fixing days ("
                       << swapIndex2_->fixingDays() << ")");

        QL_REQUIRE(swapIndex1_->fixingCalendar() ==
                       swapIndex2_->fixingCalendar(),
                   "index1 fixingCalendar ("
                       << swapIndex1_->fixingCalendar() << ")"
                       << "must be equal to index2 fixingCalendar ("
                       << swapIndex2_->fixingCalendar() << ")");

        QL_REQUIRE(swapIndex1_->currency() == swapIndex2_->currency(),
                   "index1 currency (" << swapIndex1_->currency() << ")"
                                       << "must be equal to index2 currency ("
                                       << swapIndex2_->currency() << ")");

        QL_REQUIRE(swapIndex1_->dayCounter() == swapIndex2_->dayCounter(),
                   "index1 dayCounter ("
                       << swapIndex1_->dayCounter() << ")"
                       << "must be equal to index2 dayCounter ("
                       << swapIndex2_->dayCounter() << ")");

        QL_REQUIRE(swapIndex1_->fixedLegTenor() == swapIndex2_->fixedLegTenor(),
                   "index1 fixedLegTenor ("
                       << swapIndex1_->fixedLegTenor() << ")"
                       << "must be equal to index2 fixedLegTenor ("
                       << swapIndex2_->fixedLegTenor());

        QL_REQUIRE(swapIndex1_->fixedLegConvention() ==
                       swapIndex2_->fixedLegConvention(),
                   "index1 fixedLegConvention ("
                       << swapIndex1_->fixedLegConvention() << ")"
                       << "must be equal to index2 fixedLegConvention ("
                       << swapIndex2_->fixedLegConvention());
    }
}
