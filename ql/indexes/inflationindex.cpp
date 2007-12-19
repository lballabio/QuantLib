/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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


#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {

    InflationIndex::InflationIndex(const std::string& familyName,
                                   const Region& region,
                                   bool revised,
                                   bool interpolated,
                                   Frequency frequency,
                                   const Period& availabilityLag,
                                   const Currency& currency)
    : familyName_(familyName), region_(region),
      revised_(revised), interpolated_(interpolated),
      frequency_(frequency), availabilityLag_(availabilityLag),
      currency_(currency) {
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));
    }


    std::string InflationIndex::name() const {
        return region_.name() + " " + familyName_;
    }

    Calendar InflationIndex::fixingCalendar() const {
        static NullCalendar c;
        return c;
    }

    bool InflationIndex::isValidFixingDate(const Date&) const {
        return true;
    }


    void InflationIndex::addFixing(const Date& fixingDate,
                                   Real fixing,
                                   bool forceOverwrite) {

        std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
        Size n = lim.second - lim.first + 1;
        std::vector<Date> dates(n);
        std::vector<Rate> rates(n);
        for (Size i=0; i<n; ++i) {
            dates[i] = lim.first + i;
            rates[i] = fixing;
        }

        Index::addFixings(dates.begin(), dates.end(),
                          rates.begin(), forceOverwrite);
    }


    void InflationIndex::update() {
        notifyObservers();
    }

    std::string InflationIndex::familyName() const {
        return familyName_;
    }

    Region InflationIndex::region() const {
        return region_;
    }

    bool InflationIndex::revised() const {
        return revised_;
    }

    bool InflationIndex::interpolated() const {
        return interpolated_;
    }

    Frequency InflationIndex::frequency() const {
        return frequency_;
    }

    Period InflationIndex::availabilityLag() const {
        return availabilityLag_;
    }

    Currency InflationIndex::currency() const {
        return currency_;
    }




    ZeroInflationIndex::ZeroInflationIndex(
                      const std::string& familyName,
                      const Region& region,
                      bool revised,
                      bool interpolated,
                      Frequency frequency,
                      const Period& availabilityLag,
                      const Currency& currency,
                      const Handle<ZeroInflationTermStructure>& zeroInflation)
    : InflationIndex(familyName, region, revised, interpolated,
                     frequency, availabilityLag, currency),
      zeroInflation_(zeroInflation) {
        registerWith(zeroInflation_);
    }

    Rate ZeroInflationIndex::fixing(const Date& fixingDate,
                                    bool forecastTodaysFixing) const {

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;
        std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, frequency_);
        todayMinusLag = lim.second + 1;

        if (fixingDate < todayMinusLag
            || (fixingDate == todayMinusLag && !forecastTodaysFixing)) {
            Real pastFixing =
                IndexManager::instance().getHistory(name())[fixingDate];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << name() << " fixing for " << fixingDate);
            return pastFixing;
        } else {
            return forecastFixing(fixingDate);
        }
    }


    Rate ZeroInflationIndex::forecastFixing(const Date& fixingDate) const {

        // the term structure is relative to the fixing value at the base date.
        Date baseDate = zeroInflation_->baseDate();
        Real baseFixing = fixing(baseDate);
        // get the relevant period end
        std::pair<Date,Date> limBase = inflationPeriod(baseDate, frequency_);
        Date trueBaseDate = limBase.second;

        // if the value is not interpolated, get the value for half
        // way along the period.
        Date d = fixingDate;
        if (!interpolated()) {
            std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
            Integer n = (lim.second - lim.first)/2;
            d = lim.first + n;
        }

        // Assume annual compounding (we're using a zero inflation
        // term structure)
        Rate zero = zeroInflation_->zeroRate(d);
        Time t = zeroInflation_->dayCounter().yearFraction(trueBaseDate, d);

        return baseFixing * std::pow(1.0 + zero, t);
    }


    Handle<ZeroInflationTermStructure>
    ZeroInflationIndex::zeroInflationTermStructure() const {
        return zeroInflation_;
    }




    YoYInflationIndex::YoYInflationIndex(
                        const std::string& familyName,
                        const Region& region,
                        bool revised,
                        bool interpolated,
                        bool ratio,
                        Frequency frequency,
                        const Period& availabilityLag,
                        const Currency& currency,
                        const Handle<YoYInflationTermStructure>& yoyInflation)
    : InflationIndex(familyName, region, revised, interpolated,
                     frequency, availabilityLag, currency),
      ratio_(ratio), yoyInflation_(yoyInflation) {
        registerWith(yoyInflation_);
    }


    Rate YoYInflationIndex::fixing(const Date& fixingDate,
                                   bool forecastTodaysFixing) const {

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;
        std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, frequency_);
        todayMinusLag = lim.second + 1;

        if (fixingDate < todayMinusLag
            || (fixingDate == todayMinusLag && !forecastTodaysFixing)) {
            if (ratio_) {
                Rate pastFixing =
                    IndexManager::instance().getHistory(name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for "
                           << fixingDate);
                Date previousDate = fixingDate - 1*Years;
                Rate previousFixing =
                    IndexManager::instance().getHistory(name())[previousDate];
                QL_REQUIRE(previousFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for "
                           << previousDate );
                return pastFixing/previousFixing - 1.0;
            } else {
                Rate pastFixing =
                    IndexManager::instance().getHistory(name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for "
                           << fixingDate);
                return pastFixing;
            }
        } else {
            return forecastFixing(fixingDate);
        }
    }


    Real YoYInflationIndex::forecastFixing(const Date& fixingDate) const {

        // if the value is not interpolated get the value for
        // half way along the period.
        Date d = fixingDate;
        if (!interpolated()) {
            std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
            Integer n = (lim.second-lim.first)/2;
            d = lim.first + n;
        }

        return yoyInflation_->yoyRate(d);
    }


    bool YoYInflationIndex::ratio() const {
        return ratio_;
    }

    Handle<YoYInflationTermStructure>
    YoYInflationIndex::yoyInflationTermStructure() const {
        return yoyInflation_;
    }

}

