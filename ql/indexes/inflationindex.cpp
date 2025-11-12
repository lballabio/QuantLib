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
 <https://www.quantlib.org/license.shtml>.

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
          case AsIndex:
          case Flat: {
              auto fixingPeriod = inflationPeriod(date - observationLag, index->frequency());
              return index->fixing(fixingPeriod.first);
          }
          case Linear: {
              auto fixingPeriod = inflationPeriod(date - observationLag, index->frequency());
              auto interpolationPeriod = inflationPeriod(date, index->frequency());

              auto I0 = index->fixing(fixingPeriod.first);

              if (date == interpolationPeriod.first) {
                  // special case; no interpolation.  This avoids asking for
                  // the fixing at the end of the period, which might need a
                  // forecast curve to be set.
                  return I0;
              }

              static const auto oneDay = Period(1, Days);

              auto I1 = index->fixing(fixingPeriod.second + oneDay);

              return I0 + (I1 - I0) * (date - interpolationPeriod.first) /
                  (Real)((interpolationPeriod.second + oneDay) - interpolationPeriod.first);
          }
          default:
            QL_FAIL("unknown CPI interpolation type: " << int(interpolationType));
        }
    }


    Real CPI::laggedYoYRate(const ext::shared_ptr<YoYInflationIndex>& index,
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
              if (index->ratio() && !index->needsForecast(date)) {
                  // in the case of a ratio, the convention seems to be to interpolate
                  // the underlying index fixings first, then take the ratio.  This is
                  // not the same as taking the ratios and then interpolate, which is
                  // equivalent to what the else clause does.
                  // However, we can only do this if the fixings we need are in the past,
                  // because forecasts need to be done through the YoY forecast curve,
                  // and not the underlying index.

                  auto underlying = index->underlyingIndex();
                  Rate Z1 = CPI::laggedFixing(underlying, date, observationLag, interpolationType);
                  Rate Z0 = CPI::laggedFixing(underlying, date - 1*Years, observationLag, interpolationType);

                  return Z1/Z0 - 1.0;

              } else {
                  static const auto oneDay = Period(1, Days);

                  auto fixingPeriod = inflationPeriod(date - observationLag, index->frequency());
                  auto interpolationPeriod = inflationPeriod(date, index->frequency());

                  auto Y0 = index->fixing(fixingPeriod.first);

                  if (date == interpolationPeriod.first) {
                      // special case; no interpolation anyway.
                      return Y0;
                  }

                  auto Y1 = index->fixing(fixingPeriod.second + oneDay);

                  return Y0 + (Y1 - Y0) * (date - interpolationPeriod.first) /
                      (Real)((interpolationPeriod.second + oneDay) - interpolationPeriod.first);
              }
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
        registerWith(notifier());
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
            QL_REQUIRE(I1 != Null<Real>(),
                       "Missing " << name() << " fixing for "
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

        Date today = Settings::instance().evaluationDate();

        auto latestPossibleHistoricalFixingPeriod =
            inflationPeriod(today - availabilityLag_, frequency_);

        // Zero-index fixings are always non-interpolated.
        auto fixingPeriod = inflationPeriod(fixingDate, frequency_);
        Date latestNeededDate = fixingPeriod.first;

        if (latestNeededDate < latestPossibleHistoricalFixingPeriod.first) {
            // the fixing date is well before the availability lag, so
            // we know that fixings must be provided.
            return false;
        } else if (latestNeededDate > latestPossibleHistoricalFixingPeriod.second) {
            // the fixing can't be available yet
            return true;
        } else {
            // we're not sure, but the fixing might be there so we check.
            Real f = timeSeries()[latestNeededDate];
            return (f == Null<Real>());
        }
    }


    Real ZeroInflationIndex::forecastFixing(const Date& fixingDate) const {
        // the term structure is relative to the fixing value at the base date.
        Date baseDate = zeroInflation_->baseDate();
        QL_REQUIRE(!needsForecast(baseDate),
                   name() << " index fixing at base date " << baseDate << " is not available");
        Real baseFixing = fixing(baseDate);

        std::pair<Date, Date> fixingPeriod = inflationPeriod(fixingDate, frequency_);

        Date firstDateInPeriod = fixingPeriod.first;
        Rate Z1 = zeroInflation_->zeroRate(firstDateInPeriod, false);
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
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : InflationIndex("YYR_" + underlyingIndex->familyName(), underlyingIndex->region(),
                     underlyingIndex->revised(), underlyingIndex->frequency(),
                     underlyingIndex->availabilityLag(), underlyingIndex->currency()),
      interpolated_(false), ratio_(true), underlyingIndex_(underlyingIndex),
      yoyInflation_(std::move(yoyInflation)) {
        registerWith(underlyingIndex_);
        registerWith(yoyInflation_);
    }

    YoYInflationIndex::YoYInflationIndex(const ext::shared_ptr<ZeroInflationIndex>& underlyingIndex,
                                         bool interpolated,
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : YoYInflationIndex(underlyingIndex, std::move(yoyInflation)) {
        interpolated_ = interpolated;
    }

    YoYInflationIndex::YoYInflationIndex(const std::string& familyName,
                                         const Region& region,
                                         bool revised,
                                         Frequency frequency,
                                         const Period& availabilityLag,
                                         const Currency& currency,
                                         Handle<YoYInflationTermStructure> yoyInflation)
    : InflationIndex(familyName, region, revised, frequency, availabilityLag, currency),
      interpolated_(false), ratio_(false), yoyInflation_(std::move(yoyInflation)) {
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
    : YoYInflationIndex(familyName, region, revised, frequency, availabilityLag, currency, std::move(yoyInflation)) {
        interpolated_ = interpolated;
    }


    Rate YoYInflationIndex::fixing(const Date& fixingDate,
                                   bool /*forecastTodaysFixing*/) const {
        if (needsForecast(fixingDate)) {
            return forecastFixing(fixingDate);
        } else {
            return pastFixing(fixingDate);
        }
    }

    Date YoYInflationIndex::lastFixingDate() const {
        if (ratio()) {
            return underlyingIndex_->lastFixingDate();
        } else {
            const auto& fixings = timeSeries();
            QL_REQUIRE(!fixings.empty(), "no fixings stored for " << name());
            // attribute fixing to first day of the underlying period
            return inflationPeriod(fixings.lastDate(), frequency_).first;
        }
    }

    bool YoYInflationIndex::needsForecast(const Date& fixingDate) const {
        Date today = Settings::instance().evaluationDate();

        auto fixingPeriod = inflationPeriod(fixingDate, frequency_);
        Date latestNeededDate;
        if (!interpolated() || fixingDate == fixingPeriod.first)
            latestNeededDate = fixingPeriod.first;
        else
            latestNeededDate = fixingPeriod.second + 1;

        if (ratio()) {
            return underlyingIndex_->needsForecast(latestNeededDate);
        } else {
            auto latestPossibleHistoricalFixingPeriod =
                inflationPeriod(today - availabilityLag_, frequency_);

            if (latestNeededDate < latestPossibleHistoricalFixingPeriod.first) {
                // the fixing date is well before the availability lag, so
                // we know that fixings must be provided.
                return false;
            } else if (latestNeededDate > latestPossibleHistoricalFixingPeriod.second) {
                // the fixing can't be available yet
                return true;
            } else {
                // we're not sure, but the fixing might be there so we check.
                Real f = timeSeries()[latestNeededDate];
                return (f == Null<Real>());
            }
        }
    }

    Real YoYInflationIndex::pastFixing(const Date& fixingDate) const {
        if (ratio()) {

            auto interpolationType = interpolated() ? CPI::Linear : CPI::Flat;

            Rate pastFixing = CPI::laggedFixing(underlyingIndex_, fixingDate, Period(0, Months), interpolationType);
            Rate previousFixing = CPI::laggedFixing(underlyingIndex_, fixingDate - 1*Years, Period(0, Months), interpolationType);

            return pastFixing/previousFixing - 1.0;

        } else {  // NOT ratio

            const auto& ts = timeSeries();
            auto [periodStart, periodEnd] = inflationPeriod(fixingDate, frequency_);

            Rate YY0 = ts[periodStart];
            QL_REQUIRE(YY0 != Null<Rate>(),
                       "Missing " << name() << " fixing for " << periodStart);

            if (!interpolated() || /* degenerate case */ fixingDate == periodStart) {

                return YY0;

            } else {

                Real dp = periodEnd + 1 - periodStart;
                Real dl = fixingDate - periodStart;
                Rate YY1 = ts[periodEnd+1];
                QL_REQUIRE(YY1 != Null<Rate>(),
                           "Missing " << name() << " fixing for " << periodEnd+1);
                return YY0 + (YY1 - YY0) * dl / dp;

            }
        }
    }

    Real YoYInflationIndex::forecastFixing(const Date& fixingDate) const {

        Date d;
        if (interpolated()) {
            d = fixingDate;
        } else {
            // if the value is not interpolated use the starting value
            // by internal convention this will be consistent
            std::pair<Date,Date> fixingPeriod = inflationPeriod(fixingDate, frequency_);
            d = fixingPeriod.first;
        }
        return yoyInflation_->yoyRate(d);
    }

    ext::shared_ptr<YoYInflationIndex> YoYInflationIndex::clone(
                           const Handle<YoYInflationTermStructure>& h) const {
        QL_DEPRECATED_DISABLE_WARNING
        if (ratio_) {
            // NOLINTNEXTLINE(modernize-make-shared)
            return ext::shared_ptr<YoYInflationIndex>(
                new YoYInflationIndex(underlyingIndex_, interpolated_, h));
        } else {
            // NOLINTNEXTLINE(modernize-make-shared)
            return ext::shared_ptr<YoYInflationIndex>(
                new YoYInflationIndex(familyName_, region_, revised_,
                                      interpolated_, frequency_,
                                      availabilityLag_, currency_, h));
        }
        QL_DEPRECATED_ENABLE_WARNING
    }


    CPI::InterpolationType
    detail::CPI::effectiveInterpolationType(const QuantLib::CPI::InterpolationType& type) {
        if (type == QuantLib::CPI::AsIndex) {
            return QuantLib::CPI::Flat;
        } else {
            return type;
        }
    }

    CPI::InterpolationType
    detail::CPI::effectiveInterpolationType(const QuantLib::CPI::InterpolationType& type,
                                            const ext::shared_ptr<YoYInflationIndex>& index) {
        if (type == QuantLib::CPI::AsIndex) {
            return index->interpolated() ? QuantLib::CPI::Linear : QuantLib::CPI::Flat;
        } else {
            return type;
        }
    }

    bool detail::CPI::isInterpolated(const QuantLib::CPI::InterpolationType& type) {
        return detail::CPI::effectiveInterpolationType(type) == QuantLib::CPI::Linear;
    }

    bool detail::CPI::isInterpolated(const QuantLib::CPI::InterpolationType& type,
                                     const ext::shared_ptr<YoYInflationIndex>& index) {
        return detail::CPI::effectiveInterpolationType(type, index) == QuantLib::CPI::Linear;
    }

}
