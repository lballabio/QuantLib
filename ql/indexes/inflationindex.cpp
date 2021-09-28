/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2021 Ralf Konrad Eckel

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
#include <utility>

namespace QuantLib {

    InflationIndex::InflationIndex(std::string familyName,
                                   Region region,
                                   bool revised,
                                   bool interpolated,
                                   Frequency frequency,
                                   const Period& availabilityLag,
                                   Currency currency)
    : InflationIndex(std::move(familyName),
                     std::move(region),
                     revised,
                     frequency,
                     availabilityLag,
                     std::move(currency)) {
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
        interpolated_ = interpolated;
        QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
    }

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    InflationIndex::InflationIndex(std::string familyName,
                                   Region region,
                                   bool revised,
                                   Frequency frequency,
                                   const Period& availabilityLag,
                                   Currency currency)
    : familyName_(std::move(familyName)), region_(std::move(region)), revised_(revised),
      frequency_(frequency), availabilityLag_(availabilityLag), currency_(std::move(currency)),
      interpolated_(false) {
        name_ = region_.name() + " " + familyName_;
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif


    Calendar InflationIndex::fixingCalendar() const {
        static NullCalendar c;
        return c;
    }

    void InflationIndex::addFixing(const Date& fixingDate, Real fixing, bool forceOverwrite) {

        std::pair<Date, Date> lim = inflationPeriod(fixingDate, frequency_);
        Size n = static_cast<QuantLib::Size>(lim.second - lim.first) + 1;
        std::vector<Date> dates(n);
        std::vector<Rate> rates(n);
        for (Size i = 0; i < n; ++i) {
            dates[i] = lim.first + i;
            rates[i] = fixing;
        }

        Index::addFixings(dates.begin(), dates.end(), rates.begin(), forceOverwrite);
    }


    ZeroInflationIndex::ZeroInflationIndex(const std::string& familyName,
                                           const Region& region,
                                           bool revised,
                                           bool interpolated,
                                           Frequency frequency,
                                           const Period& availabilityLag,
                                           const Currency& currency,
                                           Handle<ZeroInflationTermStructure> zeroInflation)
    : ZeroInflationIndex(familyName,
                         region,
                         revised,
                         frequency,
                         availabilityLag,
                         currency,
                         std::move(zeroInflation)) {
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
        interpolated_ = interpolated;
        QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
    }

    ZeroInflationIndex::ZeroInflationIndex(const std::string& familyName,
                                           const Region& region,
                                           bool revised,
                                           Frequency frequency,
                                           const Period& availabilityLag,
                                           const Currency& currency,
                                           Handle<ZeroInflationTermStructure> zeroInflation)
    : InflationIndex(familyName, region, revised, frequency, availabilityLag, currency),
      zeroInflation_(std::move(zeroInflation)) {
        registerWith(zeroInflation_);
    }

    Rate ZeroInflationIndex::fixing(const Date& aFixingDate, bool /*forecastTodaysFixing*/) const {
        if (!needsForecast(aFixingDate)) {
            std::pair<Date, Date> lim = inflationPeriod(aFixingDate, frequency_);
            const TimeSeries<Real>& ts = timeSeries();
            Real pastFixing = ts[lim.first];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << name() << " fixing for " << lim.first);
            Real theFixing = pastFixing;

            QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
            if (interpolated_) {
                QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER

                // fixings stored on first day of every period
                if (aFixingDate == lim.first) {
                    // we don't actually need the next fixing
                    theFixing = pastFixing;
                } else {
                    Real pastFixing2 = ts[lim.second + 1];
                    QL_REQUIRE(pastFixing2 != Null<Real>(),
                               "Missing " << name() << " fixing for " << lim.second + 1);

                    // Use lagged period for interpolation
                    std::pair<Date, Date> reference_period_lim = inflationPeriod(
                        aFixingDate + zeroInflationTermStructure()->observationLag(), frequency_);
                    // now linearly interpolate
                    Real daysInPeriod =
                        reference_period_lim.second + 1 - reference_period_lim.first;
                    theFixing = pastFixing + (pastFixing2 - pastFixing) *
                                                 (aFixingDate - lim.first) / daysInPeriod;
                }
            }
            return theFixing;
        } else {
            return forecastFixing(aFixingDate);
        }
    }


    bool ZeroInflationIndex::needsForecast(const Date& fixingDate) const {

        // Stored fixings are always non-interpolated.
        // If an interpolated fixing is required then
        // the availability lag + one inflation period
        // must have passed to use historical fixings
        // (because you need the next one to interpolate).
        // The interpolation is calculated (linearly) on demand.

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;

        Date historicalFixingKnown = inflationPeriod(todayMinusLag, frequency_).first - 1;
        Date latestNeededDate = fixingDate;

        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
        if (interpolated_) { // might need the next one too
            QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
            std::pair<Date, Date> p = inflationPeriod(fixingDate, frequency_);
            if (fixingDate > p.first)
                latestNeededDate += Period(frequency_);
        }

        if (latestNeededDate <= historicalFixingKnown) {
            // the fixing date is well before the availability lag, so
            // we know that fixings were provided.
            return false;
        } else if (latestNeededDate > today) {
            // the fixing can't be available, no matter what's in the
            // time series
            return true;
        } else {
            // we're not sure, but the fixing might be there so we
            // check.  Todo: check which fixings are not possible, to
            // avoid using fixings in the future
            Real f = timeSeries()[latestNeededDate];
            return (f == Null<Real>());
        }
    }


    Rate ZeroInflationIndex::forecastFixing(const Date& fixingDate) const {
        // the term structure is relative to the fixing value at the base date.
        Date baseDate = zeroInflation_->baseDate();
        QL_REQUIRE(!needsForecast(baseDate),
                   name() << " index fixing at base date " << baseDate << " is not available");
        Real baseFixing = fixing(baseDate);
        Date effectiveFixingDate;
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
        if (interpolated_) {
            QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
            effectiveFixingDate = fixingDate;
        } else {
            // start of period is the convention
            // so it's easier to do linear interpolation on fixings
            effectiveFixingDate = inflationPeriod(fixingDate, frequency()).first;
        }

        // no observation lag because it is the fixing for the date
        // but if index is not interpolated then that fixing is constant
        // for each period, hence the t uses the effectiveFixingDate
        // However, it's slightly safe to get the zeroRate with the
        // fixingDate to avoid potential problems at the edges of periods
        Time t = zeroInflation_->dayCounter().yearFraction(baseDate, effectiveFixingDate);
        bool forceLinearInterpolation = false;
        Rate zero = zeroInflation_->zeroRate(fixingDate, Period(0, Days), forceLinearInterpolation);
        // Annual compounding is the convention for zero inflation rates (or quotes)
        return baseFixing * std::pow(1.0 + zero, t);
    }


    ext::shared_ptr<ZeroInflationIndex>
    ZeroInflationIndex::clone(const Handle<ZeroInflationTermStructure>& h) const {
        /* using the new constructor and set interpolated to avoid the deprecated warning and
         * error...  */
        auto clonedIndex = ext::make_shared<ZeroInflationIndex>(
            familyName_, region_, revised_, frequency_, availabilityLag_, currency_, h);
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
        clonedIndex->interpolated_ = interpolated_;
        QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
        return clonedIndex;
    }

    // these still need to be fixed to latest versions

    YoYInflationIndex::YoYInflationIndex(const std::string& familyName,
                                         const Region& region,
                                         bool revised,
                                         bool interpolated,
                                         bool ratio,
                                         Frequency frequency,
                                         const Period& availabilityLag,
                                         const Currency& currency,
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : InflationIndex(familyName, region, revised, frequency, availabilityLag, currency),
      ratio_(ratio), yoyInflation_(std::move(yoyInflation)) {
        interpolated_ = interpolated;
        registerWith(yoyInflation_);
    }

    Rate YoYInflationIndex::fixing(const Date& fixingDate, bool /*forecastTodaysFixing*/) const {

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;
        std::pair<Date, Date> lim = inflationPeriod(todayMinusLag, frequency_);
        Date lastFix = lim.first - 1;

        Date flatMustForecastOn = lastFix + 1;
        Date interpMustForecastOn = lastFix + 1 - Period(frequency_);


        if (interpolated() && fixingDate >= interpMustForecastOn) {
            return forecastFixing(fixingDate);
        }

        if (!interpolated() && fixingDate >= flatMustForecastOn) {
            return forecastFixing(fixingDate);
        }

        // four cases with ratio() and interpolated()

        const TimeSeries<Real>& ts = timeSeries();
        if (ratio()) {

            if (interpolated()) { // IS ratio, IS interpolated

                std::pair<Date, Date> lim = inflationPeriod(fixingDate, frequency_);
                Date fixMinus1Y = NullCalendar().advance(fixingDate, -1 * Years, ModifiedFollowing);
                std::pair<Date, Date> limBef = inflationPeriod(fixMinus1Y, frequency_);
                Real dp = lim.second + 1 - lim.first;
                Real dpBef = limBef.second + 1 - limBef.first;
                Real dl = fixingDate - lim.first;
                // potentially does not work on 29th Feb
                Real dlBef = fixMinus1Y - limBef.first;
                // get the four relevant fixings
                // recall that they are stored flat for every day
                Rate limFirstFix = ts[lim.first];
                QL_REQUIRE(limFirstFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << lim.first);
                Rate limSecondFix = ts[lim.second + 1];
                QL_REQUIRE(limSecondFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << lim.second + 1);
                Rate limBefFirstFix = ts[limBef.first];
                QL_REQUIRE(limBefFirstFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << limBef.first);
                Rate limBefSecondFix =
                    IndexManager::instance().getHistory(name())[limBef.second + 1];
                QL_REQUIRE(limBefSecondFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << limBef.second + 1);

                Real linearNow = limFirstFix + (limSecondFix - limFirstFix) * dl / dp;
                Real linearBef =
                    limBefFirstFix + (limBefSecondFix - limBefFirstFix) * dlBef / dpBef;
                Rate wasYES = linearNow / linearBef - 1.0;

                return wasYES;

            } else { // IS ratio, NOT interpolated
                Rate pastFixing = ts[fixingDate];
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for " << fixingDate);
                Date previousDate = fixingDate - 1 * Years;
                Rate previousFixing = ts[previousDate];
                QL_REQUIRE(previousFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for " << previousDate);

                return pastFixing / previousFixing - 1.0;
            }

        } else { // NOT ratio

            if (interpolated()) { // NOT ratio, IS interpolated

                std::pair<Date, Date> lim = inflationPeriod(fixingDate, frequency_);
                Real dp = lim.second + 1 - lim.first;
                Real dl = fixingDate - lim.first;
                Rate limFirstFix = ts[lim.first];
                QL_REQUIRE(limFirstFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << lim.first);
                Rate limSecondFix = ts[lim.second + 1];
                QL_REQUIRE(limSecondFix != Null<Rate>(),
                           "Missing " << name() << " fixing for " << lim.second + 1);
                Real linearNow = limFirstFix + (limSecondFix - limFirstFix) * dl / dp;

                return linearNow;

            } else { // NOT ratio, NOT interpolated
                     // so just flat

                Rate pastFixing = ts[fixingDate];
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for " << fixingDate);
                return pastFixing;
            }
        }

        // QL_FAIL("YoYInflationIndex::fixing, should never get here");
    }


    Real YoYInflationIndex::forecastFixing(const Date& fixingDate) const {

        Date d;
        if (interpolated()) {
            d = fixingDate;
        } else {
            // if the value is not interpolated use the starting value
            // by internal convention this will be consistent
            std::pair<Date, Date> lim = inflationPeriod(fixingDate, frequency_);
            d = lim.first;
        }
        return yoyInflation_->yoyRate(d, 0 * Days);
    }

    ext::shared_ptr<YoYInflationIndex>
    YoYInflationIndex::clone(const Handle<YoYInflationTermStructure>& h) const {
        return ext::make_shared<YoYInflationIndex>(familyName(), region(), revised(),
                                                   interpolated(), ratio(), frequency(),
                                                   availabilityLag(), currency(), h);
    }


    CPI::InterpolationType
    detail::CPI::effectiveInterpolationType(const ext::shared_ptr<ZeroInflationIndex>& index,
                                            const QuantLib::CPI::InterpolationType& type) {
        QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_METHOD
        if (type == QuantLib::CPI::AsIndex) {
            return index->interpolated() ? QuantLib::CPI::Linear : QuantLib::CPI::Flat;
        } else {
            return type;
        }
        QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_METHOD
    }
}
