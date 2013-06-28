/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Piero Del Boca
 Copyright (C) 2009 Chris Kenyon

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


#include <ql/termstructures/inflation/seasonality.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    bool Seasonality::isConsistent(const InflationTermStructure&) const {
        return true;
    }


    //Multiplicative Seasonality on price = on CPI/RPI/HICP/etc
    //
    MultiplicativePriceSeasonality::MultiplicativePriceSeasonality() {
    }


    void MultiplicativePriceSeasonality::validate() const
    {
        switch (this->frequency()) {
            case Semiannual:        //2
            case EveryFourthMonth:  //3
            case Quarterly:         //4
            case Bimonthly:         //6
            case Monthly:           //12
            case Biweekly:          // etc.
            case Weekly:
            case Daily:
                QL_REQUIRE( (this->seasonalityFactors().size() %
                             this->frequency()) == 0,
                           "For frequency " << this->frequency()
                           << " require multiple of " << ((int)this->frequency()) << " factors "
                           << this->seasonalityFactors().size() << " were given.");
            break;
            default:
                QL_FAIL("bad frequency specified: " << this->frequency()
                        << ", only semi-annual through daily permitted.");
            break;
        }
    }


    bool MultiplicativePriceSeasonality::isConsistent(const InflationTermStructure& iTS) const
    {
        // If multi-year is the specification consistent with the term structure start date?
        // We do NOT test daily seasonality because this will, in general, never be consistent
        // given weekends, holidays, leap years, etc.
        if(this->frequency() == Daily) return true;
        if(Size(this->frequency()) == seasonalityFactors().size()) return true;

        // how many years do you need to test?
        Size nTest = seasonalityFactors().size() / this->frequency();
        // ... relative to the start of the inflation curve
        std::pair<Date,Date> lim = inflationPeriod(iTS.baseDate(), iTS.frequency());
        Date curveBaseDate = lim.second;
        Real factorBase = this->seasonalityFactor(curveBaseDate);

        Real eps = 0.00001;
        for (Size i = 1; i < nTest; i++) {
            Real factorAt = this->seasonalityFactor(curveBaseDate+Period(i,Years));
            QL_REQUIRE(std::fabs(factorAt-factorBase)<eps,"seasonality is inconsistent with inflation term structure, factors "
                       << factorBase << " and later factor " << factorAt << ", " << i << " years later from inflation curve "
                       <<" with base date at " << curveBaseDate);
        }

        return true;
    }


    MultiplicativePriceSeasonality::MultiplicativePriceSeasonality(const Date& seasonalityBaseDate, const Frequency frequency,
                                                                   const std::vector<Rate> seasonalityFactors)
    {
        set(seasonalityBaseDate, frequency, seasonalityFactors);
    }

    void MultiplicativePriceSeasonality::set(const Date& seasonalityBaseDate, const Frequency frequency,
                                             const std::vector<Rate> seasonalityFactors)
    {
        frequency_ = frequency;
        seasonalityFactors_ = std::vector<Rate>(seasonalityFactors.size());
        for(Size i=0; i<seasonalityFactors.size(); i++) {
            seasonalityFactors_[i] = seasonalityFactors[i];
        }
        seasonalityBaseDate_ = seasonalityBaseDate;
        validate();
    }

    Date MultiplicativePriceSeasonality::seasonalityBaseDate() const {
        return seasonalityBaseDate_;
    }

    Frequency MultiplicativePriceSeasonality::frequency() const {
        return frequency_;
    }

    std::vector<Rate> MultiplicativePriceSeasonality::seasonalityFactors() const {
        return seasonalityFactors_;
    }


    Rate MultiplicativePriceSeasonality::correctZeroRate(const Date &d,
                                                         const Rate r,
                                                         const InflationTermStructure& iTS) const {
        std::pair<Date,Date> lim = inflationPeriod(iTS.baseDate(), iTS.frequency());
        Date curveBaseDate = lim.second;
        return seasonalityCorrection(r, d, iTS.dayCounter(), curveBaseDate, true);
    }


    Rate MultiplicativePriceSeasonality::correctYoYRate(const Date &d,
                                                        const Rate r,
                                                        const InflationTermStructure& iTS) const {
        std::pair<Date,Date> lim = inflationPeriod(iTS.baseDate(), iTS.frequency());
        Date curveBaseDate = lim.second;
        return seasonalityCorrection(r, d, iTS.dayCounter(), curveBaseDate, false);
    }


    Real MultiplicativePriceSeasonality::seasonalityFactor(const Date &to) const {

        Date from = seasonalityBaseDate();
        Frequency factorFrequency = frequency();
        Size nFactors = seasonalityFactors().size();
        Period factorPeriod(factorFrequency);
        Size which = 0;
        if (from==to) {
            which = 0;
        } else {
            // days, weeks, months, years are the only time unit possibilities
            Integer diffDays = std::abs(to - from);  // in days
            Integer dir = 1;
            if(from > to)dir = -1;
            Integer diff;
            if (factorPeriod.units() == Days) {
                diff = dir*diffDays;
            } else if (factorPeriod.units() == Weeks) {
                diff = dir * (diffDays / 7);
            } else if (factorPeriod.units() == Months) {
                std::pair<Date,Date> lim = inflationPeriod(to, factorFrequency);
                diff = diffDays / (31*factorPeriod.length());
                Date go = from + dir*diff*factorPeriod;
                while ( !(lim.first <= go && go <= lim.second) ) {
                    go += dir*factorPeriod;
                    diff++;
                }
                diff=dir*diff;
            } else if (factorPeriod.units() == Years) {
                QL_FAIL("seasonality period time unit is not allowed to be : " << factorPeriod.units());
            } else {
                QL_FAIL("Unknown time unit: " << factorPeriod.units());
            }
            // now adjust to the available number of factors, direction dependent

            if (dir==1) {
                which = diff % nFactors;
            } else {
                which = (nFactors - (-diff % nFactors)) % nFactors;
            }
        }

        return seasonalityFactors()[which];
    }


    Rate MultiplicativePriceSeasonality::seasonalityCorrection(Rate rate,
                                                               const Date& atDate,
                                                               const DayCounter& dc,
                                                               const Date& curveBaseDate,
                                                               const bool isZeroRate) const {
        // need _two_ corrections in order to get: seasonality = factor[atDate-seasonalityBase] / factor[reference-seasonalityBase]
        // i.e. for ZERO inflation rates you have the true fixing at the curve base so this factor must be normalized to one
        //      for YoY inflation rates your reference point is the year before

        Real factorAt = this->seasonalityFactor(atDate);

        //Getting seasonality correction for either ZC or YoY
        Rate f;
        if (isZeroRate) {
            Rate factorBase = this->seasonalityFactor(curveBaseDate);
            Real seasonalityAt = factorAt / factorBase;
            Time timeFromCurveBase = dc.yearFraction(curveBaseDate, atDate);
            f = std::pow(seasonalityAt, 1/timeFromCurveBase);
        }
        else {
            Rate factor1Ybefore = this->seasonalityFactor(atDate - Period(1,Years));
            f = factorAt / factor1Ybefore;
        }

        return (rate + 1)*f - 1;
    }

}






