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

    Real CPI::laggedFixing(const ext::shared_ptr<ZeroInflationIndex>& index,
                           const Date& date,
                           const Period& observationLag,
                           CPI::InterpolationType interpolationType) {

        switch (interpolationType) {
          case AsIndex: {
              return index->fixing(date - observationLag);
          }
          case Flat: {
              auto fixingPeriod = inflationPeriod(date - observationLag, index->frequency());
              return index->fixing(fixingPeriod.first);
          }
          case Linear: {
              auto fixingPeriod = inflationPeriod(date - observationLag, index->frequency());
              auto interpolationPeriod = inflationPeriod(date, index->frequency());

              if (date == interpolationPeriod.first) {
                  // special case; no interpolation.  This avoids asking for
                  // the fixing at the end of the period, which might need a
                  // forecast curve to be set.
                  return index->fixing(fixingPeriod.first);
              }

              static const auto oneDay = Period(1, Days);

              auto I0 = index->fixing(fixingPeriod.first);
              auto I1 = index->fixing(fixingPeriod.second + oneDay);

              return I0 + (I1 - I0) * (date - interpolationPeriod.first) /
                  (Real)((interpolationPeriod.second + oneDay) - interpolationPeriod.first);
          }
          default:
            QL_FAIL("unknown CPI interpolation type: " << int(interpolationType));
        }
    }


    InflationIndex::InflationIndex(std::string familyName,
                                   Region region,
                                   bool revised,
                                   Frequency frequency,
                                   const Period& availabilityLag,
                                   Currency currency)
    : familyName_(std::move(familyName)), region_(std::move(region)), revised_(revised),
      frequency_(frequency), availabilityLag_(availabilityLag), currency_(std::move(currency)) {
        name_ = region_.name() + " " + familyName_;
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(InflationIndex::name()));
    }

    Calendar InflationIndex::fixingCalendar() const {
        static NullCalendar c;
        return c;
    }

    void InflationIndex::addFixing(const Date& fixingDate,
                                   Real fixing,
                                   bool forceOverwrite) {

        std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
        Size n = static_cast<QuantLib::Size>(lim.second - lim.first) + 1;
        std::vector<Date> dates(n);
        std::vector<Rate> rates(n);
        for (Size i=0; i<n; ++i) {
            dates[i] = lim.first + i;
            rates[i] = fixing;
        }

        Index::addFixings(dates.begin(), dates.end(),
                          rates.begin(), forceOverwrite);
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

    Real ZeroInflationIndex::fixing(const Date& fixingDate,
                                    bool /*forecastTodaysFixing*/) const {
        if (!needsForecast(fixingDate)) {
            const Real I1 = pastFixing(fixingDate);
            QL_REQUIRE(I1 != Null<Real>(), "Missing "
                                               << name() << " fixing for "
                                               << inflationPeriod(fixingDate, frequency_).first);

            return I1;
        } else {
            return forecastFixing(fixingDate);
        }
    }

    Real ZeroInflationIndex::pastFixing(const Date& fixingDate) const {
        const auto p = inflationPeriod(fixingDate, frequency_);
        const auto& ts = timeSeries();
        return ts[p.first];
    }

    Date ZeroInflationIndex::lastFixingDate() const {
        const auto& fixings = timeSeries();
        QL_REQUIRE(!fixings.empty(), "no fixings stored for " << name());
        // attribute fixing to first day of the underlying period
        return inflationPeriod(fixings.lastDate(), frequency_).first;
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

        Date historicalFixingKnown =
            inflationPeriod(todayMinusLag, frequency_).first-1;
        Date latestNeededDate = fixingDate;

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
            Date first = Date(1, latestNeededDate.month(), latestNeededDate.year());
            Real f = timeSeries()[first];
            return (f == Null<Real>());
        }
    }


    Real ZeroInflationIndex::forecastFixing(const Date& fixingDate) const {
        // the term structure is relative to the fixing value at the base date.
        Date baseDate = zeroInflation_->baseDate();
        QL_REQUIRE(!needsForecast(baseDate),
                   name() << " index fixing at base date " << baseDate << " is not available");
        Real baseFixing = fixing(baseDate);

        std::pair<Date, Date> p = inflationPeriod(fixingDate, frequency_);

        Date firstDateInPeriod = p.first;
        Rate Z1 = zeroInflation_->zeroRate(firstDateInPeriod, Period(0,Days), false);
        Time t1 = inflationYearFraction(frequency_, false, zeroInflation_->dayCounter(),
                                        baseDate, firstDateInPeriod);
        return baseFixing * std::pow(1.0 + Z1, t1);
    }


    ext::shared_ptr<ZeroInflationIndex> ZeroInflationIndex::clone(
                          const Handle<ZeroInflationTermStructure>& h) const {
        return ext::make_shared<ZeroInflationIndex>(
            familyName_, region_, revised_, frequency_, availabilityLag_, currency_, h);
    }


    YoYInflationIndex::YoYInflationIndex(const ext::shared_ptr<ZeroInflationIndex>& underlyingIndex,
                                         bool interpolated,
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : InflationIndex("YYR_" + underlyingIndex->familyName(), underlyingIndex->region(),
                     underlyingIndex->revised(), underlyingIndex->frequency(),
                     underlyingIndex->availabilityLag(), underlyingIndex->currency()),
      interpolated_(interpolated), ratio_(true), underlyingIndex_(underlyingIndex),
      yoyInflation_(std::move(yoyInflation)) {
        registerWith(underlyingIndex_);
        registerWith(yoyInflation_);
    }

    YoYInflationIndex::YoYInflationIndex(const std::string& familyName,
                                         const Region& region,
                                         bool revised,
                                         bool interpolated,
                                         Frequency frequency,
                                         const Period& availabilityLag,
                                         const Currency& currency,
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : InflationIndex(familyName, region, revised, frequency, availabilityLag, currency),
      interpolated_(interpolated), ratio_(false), yoyInflation_(std::move(yoyInflation)) {
        registerWith(yoyInflation_);
    }


    Rate YoYInflationIndex::fixing(const Date& fixingDate,
                                   bool /*forecastTodaysFixing*/) const {

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;
        std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, frequency_);
        Date lastFix = lim.first-1;

        Date flatMustForecastOn = lastFix+1;
        Date interpMustForecastOn = lastFix+1 - Period(frequency_);

        if (interpolated() && fixingDate >= interpMustForecastOn) {
            return forecastFixing(fixingDate);
        }

        if (!interpolated() && fixingDate >= flatMustForecastOn) {
            return forecastFixing(fixingDate);
        }

        if (ratio()) {

            auto interpolationType = interpolated() ? CPI::Linear : CPI::Flat;

            Rate pastFixing = CPI::laggedFixing(underlyingIndex_, fixingDate, Period(0, Months), interpolationType);
            Rate previousFixing = CPI::laggedFixing(underlyingIndex_, fixingDate - 1*Years, Period(0, Months), interpolationType);

            return pastFixing/previousFixing - 1.0;

        } else {  // NOT ratio

            if (interpolated()) { // NOT ratio, IS interpolated
                const TimeSeries<Real>& ts = timeSeries();

                std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
                Real dp = lim.second + 1 - lim.first;
                Real dl = fixingDate - lim.first;
                Rate limFirstFix = ts[lim.first];
                QL_REQUIRE(limFirstFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.first );
                Rate limSecondFix = ts[lim.second+1];
                QL_REQUIRE(limSecondFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.second+1 );
                Real linearNow = limFirstFix + (limSecondFix-limFirstFix)*dl/dp;

                return linearNow;

            } else { // NOT ratio, NOT interpolated
                     // so just flat

                Rate pastFixing = this->pastFixing(fixingDate);
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for "
                                      << inflationPeriod(fixingDate, frequency_).first);
                return pastFixing;

            }
        }
    }

    Real YoYInflationIndex::pastFixing(const Date& fixingDate) const {
        if (!ratio() && !interpolated()) {
            const auto& ts = timeSeries();
            std::pair<Date, Date> lim = inflationPeriod(fixingDate, frequency_);
            return ts[lim.first];
        }
        QL_FAIL("pastFixing is only supported for non-ratio and non-interpolated YOY indices");
    }

    Real YoYInflationIndex::forecastFixing(const Date& fixingDate) const {

        Date d;
        if (interpolated()) {
            d = fixingDate;
        } else {
            // if the value is not interpolated use the starting value
            // by internal convention this will be consistent
            std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
            d = lim.first;
        }
        return yoyInflation_->yoyRate(d,0*Days);
    }

    ext::shared_ptr<YoYInflationIndex> YoYInflationIndex::clone(
                           const Handle<YoYInflationTermStructure>& h) const {
        if (ratio_) {
            return ext::make_shared<YoYInflationIndex>(underlyingIndex_, interpolated_, h);
        } else {
            return ext::make_shared<YoYInflationIndex>(familyName_, region_, revised_,
                                                       interpolated_, frequency_,
                                                       availabilityLag_, currency_, h);
        }
    }


    CPI::InterpolationType
    detail::CPI::effectiveInterpolationType(const QuantLib::CPI::InterpolationType& type) {
        if (type == QuantLib::CPI::AsIndex) {
            return QuantLib::CPI::Flat;
        } else {
            return type;
        }
    }

}
