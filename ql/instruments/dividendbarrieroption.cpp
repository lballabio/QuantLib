/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    DividendBarrierOption::DividendBarrierOption(
                        Barrier::Type barrierType,
                        Real barrier,
                        Real rebate,
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        const ext::shared_ptr<Exercise>& exercise,
                        const std::vector<Date>& dividendDates,
                        const std::vector<Real>& dividends)
    : BarrierOption(barrierType, barrier, rebate, payoff, exercise),
      cashFlow_(DividendVector(dividendDates, dividends)) {}


    void DividendBarrierOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        BarrierOption::setupArguments(args);

        DividendBarrierOption::arguments* arguments =
            dynamic_cast<DividendBarrierOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong engine type");

        arguments->cashFlow = cashFlow_;
    }


    void DividendBarrierOption::arguments::validate() const {
        BarrierOption::arguments::validate();

        Date exerciseDate = exercise->lastDate();

        for (Size i = 0; i < cashFlow.size(); i++) {
            QL_REQUIRE(cashFlow[i]->date() <= exerciseDate,
                       "the " << io::ordinal(i+1) << " dividend date ("
                       << cashFlow[i]->date()
                       << ") is later than the exercise date ("
                       << exerciseDate << ")");
        }
    }

}

