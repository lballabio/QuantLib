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

    Rate ZeroInflationIndex::fixing(const Date& aFixingDate,
                                    bool /*forecastTodaysFixing*/) const {

        // Stored fixings are always non-interpolated.
        // If an interpolated fixing is required then
        // the availability lag + one inflation period
        // must have passsed to use historical fixings
        // (because you need the next one to interpolate).
        // The interpolation is calculated (linearly) on demand.

        Date today = Settings::instance().evaluationDate();
        Date todayMinusLag = today - availabilityLag_;

        std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, frequency_);
        Date historicalFixingKnown = lim.first-1;
        Date fixingDateNeeded = aFixingDate;
        if (interpolated_) { // need the next one too
            fixingDateNeeded = fixingDateNeeded + Period(frequency_);
        }

        if (fixingDateNeeded <= historicalFixingKnown) {

            Real pastFixing =
                IndexManager::instance().getHistory(name())[aFixingDate];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << name() << " fixing for " << aFixingDate);
            Real theFixing = pastFixing;

            if (interpolated_) {
                // fixings stored flat & for every day
                Date fixingDate2 = aFixingDate + Period(frequency_);
                Real pastFixing2 =
                    IndexManager::instance().getHistory(name())[fixingDate2];
                QL_REQUIRE(pastFixing2 != Null<Real>(),
                           "Missing " << name() << " fixing for " << fixingDate2);
                // now linearly interpolate
                std::pair<Date,Date> lim = inflationPeriod(aFixingDate, frequency_);
                Real daysInPeriod = lim.second+1 - lim.first;
                theFixing = pastFixing
                    + (pastFixing2-pastFixing)*(aFixingDate-lim.first)/daysInPeriod;
            }
            return theFixing;
        } else {
            return forecastFixing(aFixingDate);
        }
    }


    Rate ZeroInflationIndex::forecastFixing(const Date& fixingDate) const {

        // the term structure is relative to the fixing value at the base date.
        Date baseDate = zeroInflation_->baseDate();
        Real baseFixing = fixing(baseDate);
        Date effectiveFixingDate;
        if (interpolated()) {
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
        Rate zero = zeroInflation_->zeroRate(fixingDate, Period(0,Days), forceLinearInterpolation);
        // Annual compounding is the convention for zero inflation rates (or quotes)
        return baseFixing * std::pow(1.0 + zero, t);
    }


    Handle<ZeroInflationTermStructure>
    ZeroInflationIndex::zeroInflationTermStructure() const {
        return zeroInflation_;
    }


    boost::shared_ptr<ZeroInflationIndex> ZeroInflationIndex::clone(
                          const Handle<ZeroInflationTermStructure>& h) const {
        return boost::shared_ptr<ZeroInflationIndex>(
                      new ZeroInflationIndex(familyName_, region_, revised_,
                                             interpolated_, frequency_,
                                             availabilityLag_, currency_, h));
    }

    // these still need to be fixed to latest versions

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

        // four cases with ratio() and interpolated()

        if (ratio()) {

            if(interpolated()){ // IS ratio, IS interpolated

                std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
                Date fixMinus1Y=NullCalendar().advance(fixingDate, -1*Years, ModifiedFollowing);
                std::pair<Date,Date> limBef = inflationPeriod(fixMinus1Y, frequency_);
                Real dp= lim.second + 1 - lim.first;
                Real dpBef=limBef.second + 1 - limBef.first;
                Real dl = fixingDate-lim.first;
                // potentially does not work on 29th Feb
                Real dlBef = fixMinus1Y - limBef.first;
                // get the four relevant fixings
                // recall that they are stored flat for every day
                Rate limFirstFix =
                IndexManager::instance().getHistory(name())[lim.first];
                QL_REQUIRE(limFirstFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.first );
                Rate limSecondFix =
                IndexManager::instance().getHistory(name())[lim.second+1];
                QL_REQUIRE(limSecondFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.second+1 );
                Rate limBefFirstFix =
                IndexManager::instance().getHistory(name())[limBef.first];
                QL_REQUIRE(limBefFirstFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << limBef.first );
                Rate limBefSecondFix =
                IndexManager::instance().getHistory(name())[limBef.second+1];
                QL_REQUIRE(limBefSecondFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << limBef.second+1 );

                Real linearNow = limFirstFix + (limSecondFix-limFirstFix)*dl/dp;
                Real linearBef = limBefFirstFix + (limBefSecondFix-limBefFirstFix)*dlBef/dpBef;
                Rate wasYES = linearNow / linearBef - 1.0;

                return wasYES;

            } else {    // IS ratio, NOT interpolated
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
            }

        } else {  // NOT ratio

            if (interpolated()) { // NOT ratio, IS interpolated

                std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
                Real dp= lim.second + 1 - lim.first;
                Real dl = fixingDate-lim.first;
                Rate limFirstFix =
                IndexManager::instance().getHistory(name())[lim.first];
                QL_REQUIRE(limFirstFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.first );
                Rate limSecondFix =
                IndexManager::instance().getHistory(name())[lim.second+1];
                QL_REQUIRE(limSecondFix != Null<Rate>(),
                            "Missing " << name() << " fixing for "
                            << lim.second+1 );
                Real linearNow = limFirstFix + (limSecondFix-limFirstFix)*dl/dp;

                return linearNow;

            } else { // NOT ratio, NOT interpolated
                    // so just flat

                Rate pastFixing =
                    IndexManager::instance().getHistory(name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<Rate>(),
                           "Missing " << name() << " fixing for "
                           << fixingDate);
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
            std::pair<Date,Date> lim = inflationPeriod(fixingDate, frequency_);
            d = lim.first;
        }
        return yoyInflation_->yoyRate(d,0*Days);
    }


    bool YoYInflationIndex::ratio() const {
        return ratio_;
    }

    Handle<YoYInflationTermStructure>
    YoYInflationIndex::yoyInflationTermStructure() const {
        return yoyInflation_;
    }

    boost::shared_ptr<YoYInflationIndex> YoYInflationIndex::clone(
                           const Handle<YoYInflationTermStructure>& h) const {
        return boost::shared_ptr<YoYInflationIndex>(
                      new YoYInflationIndex(familyName_, region_, revised_,
                                            interpolated_, ratio_, frequency_,
                                            availabilityLag_, currency_, h));
    }

}

