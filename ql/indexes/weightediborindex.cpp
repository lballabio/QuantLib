/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

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

#include <ql/indexes/weightediborindex.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace QuantLib {

    const ext::shared_ptr<IborIndex>&
    WeightedIborIndex::firstComponent(const std::vector<Component>& components) {
        QL_REQUIRE(!components.empty(), "no components given for weighted index");
        return components.front().first;
    }

    Period WeightedIborIndex::longestTenor(const std::vector<Component>& components) {
        QL_REQUIRE(!components.empty(), "no components given for weighted index");
        const auto longest = std::max_element(
            components.begin(), components.end(),
            [](const auto& a, const auto& b) { return a.first->tenor() < b.first->tenor(); });
        return longest->first->tenor();
    }


    WeightedIborIndex::WeightedIborIndex(std::vector<Component> components)
    : IborIndex("Weighted",
                longestTenor(components),
                firstComponent(components)->fixingDays(),
                firstComponent(components)->currency(),
                firstComponent(components)->fixingCalendar(),
                firstComponent(components)->businessDayConvention(),
                firstComponent(components)->endOfMonth(),
                firstComponent(components)->dayCounter()),
      components_(std::move(components)) {

        /* The combination fixes on a single date and its clients derive
           dates from a single set of conventions.
        */
        const auto& first = components_.front().first;
        for (const auto& component : components_) {
            const auto& index = component.first;
            QL_REQUIRE(index, "null component index");
            QL_REQUIRE(index->fixingDays() == first->fixingDays(),
                       index->name() << " fixing days (" << index->fixingDays()
                                     << ") do not match " << first->name() << " ("
                                     << first->fixingDays() << ")");
            QL_REQUIRE(index->fixingCalendar() == first->fixingCalendar(),
                       index->name() << " fixing calendar does not match " << first->name());
            QL_REQUIRE(index->dayCounter() == first->dayCounter(),
                       index->name() << " day counter does not match " << first->name());
            QL_REQUIRE(index->currency() == first->currency(),
                       index->name() << " currency does not match " << first->name());
            registerWith(index);
        }

        std::ostringstream out;
        out << std::setprecision(4) << std::fixed;
        for (Size i = 0; i < components_.size(); ++i) {
            if (i > 0)
                out << " + ";
            out << components_[i].first->name() << "(" << components_[i].second << ")";
        }
        name_ = out.str();
        registerWith(notifier());
    }

    Date WeightedIborIndex::maturityDate(const Date& valueDate) const {
        Date maturity = Date::minDate();
        for (const auto& component : components_)
            maturity = std::max(maturity, component.first->maturityDate(valueDate));
        return maturity;
    }

    Rate WeightedIborIndex::forecastFixing(const Date& fixingDate) const {
        // as in SwapSpreadIndex, this also handles the case where one of the
        // components has a historic fixing on the evaluation date
        Rate fixing = 0.0;
        for (const auto& component : components_)
            fixing += component.second * component.first->fixing(fixingDate, false);
        return fixing;
    }

    Rate WeightedIborIndex::pastFixing(const Date& fixingDate) const {
        Rate fixing = 0.0;
        for (const auto& component : components_) {
            const Rate componentFixing = component.first->pastFixing(fixingDate);
            // a missing component means a missing fixing for the combination
            if (componentFixing == Null<Real>())
                return Null<Real>();
            fixing += component.second * componentFixing;
        }
        return fixing;
    }

    ext::shared_ptr<IborIndex> WeightedIborIndex::clone(const Handle<YieldTermStructure>&) const {
        QL_FAIL("WeightedIborIndex cannot be relinked to a single forwarding curve; "
                "relink its components instead");
    }

}
