
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/dividendvanillaoption.hpp>

namespace QuantLib {

    DividendVanillaOption::DividendVanillaOption(
        const boost::shared_ptr<BlackScholesStochasticProcess>& stochProc,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const std::vector<Date>& dividendDates,
        const std::vector<double>& dividends,
        const boost::shared_ptr<PricingEngine>& engine)
    : VanillaOption(stochProc, payoff, exercise, engine),
      dividendDates_(dividendDates), dividends_(dividends) {}

    void DividendVanillaOption::setupArguments(Arguments* args) const {
        VanillaOption::setupArguments(args);

        DividendVanillaOption::arguments* arguments =
            dynamic_cast<DividendVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong engine type");

        arguments->dividendDates = dividendDates_;
        arguments->dividends = dividends_;
    }


    void DividendVanillaOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        VanillaOption::arguments copy = *this;
        copy.validate();
        #else
        VanillaOption::arguments::validate();
        #endif

        QL_REQUIRE(dividends.size() == dividendDates.size(),
                   "the number of dividends is different from "
                   "the number of dates");

        Date exerciseDate = exercise->lastDate();

        for (Size i = 0; i < dividends.size(); i++) {
            QL_REQUIRE(dividendDates[i] <= exerciseDate,
                       "the " + SizeFormatter::toOrdinal(i) +
                       " dividend date (" +
                       DateFormatter::toString(dividendDates[i]) +
                       ") is later than the exercise date (" +
                       DateFormatter::toString(exerciseDate) + 
                       ")");
        }
    }

}

