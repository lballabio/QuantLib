/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(void*) {}
    }


    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,   // <= index availability lag
        const Date& maturity,
        const Calendar& calendar,   // index may have null calendar as valid on every day
        BusinessDayConvention paymentConvention,
        const DayCounter& dayCounter,
        const boost::shared_ptr<ZeroInflationIndex>& zii)
    : BootstrapHelper<ZeroInflationTermStructure>(quote),
    swapObsLag_(swapObsLag), maturity_(maturity), calendar_(calendar),
    paymentConvention_(paymentConvention), dayCounter_(dayCounter),
    zii_(zii) {

        if (zii_->interpolated()) {
            // if interpolated then simple
            earliestDate_ = maturity_ - swapObsLag_;
            latestDate_ = maturity_ - swapObsLag_;
        } else {
            // but if NOT interpolated then the value is valid
            // for every day in an inflation period so you actually
            // get an extended validity, however for curve building
            // just put the first date because using that convention
            // for the base date throughout
            std::pair<Date,Date> limStart = inflationPeriod(maturity_ - swapObsLag_,
                                                            zii_->frequency());
            earliestDate_ = limStart.first;
            latestDate_ = limStart.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // it's interpolation (assuming the start day is spot)
        if (zii_->interpolated()) {
            Period pShift(zii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift > zii_->availabilityLag(),
                       "inconsistency between swap observation of index "
                       << swapObsLag_ <<
                       " index availability " << zii_->availabilityLag() <<
                       " index period " << pShift <<
                       " and index availability " << zii_->availabilityLag() <<
                       " need (obsLag-index period) > availLag");
        }

        registerWith(Settings::instance().evaluationDate());
    }

    Real ZeroCouponInflationSwapHelper::impliedQuote() const {
        // what does the term structure imply?
        // in this case just the same value ... trivial case
        // (would not be so for an inflation-linked bond)
        zciis_->recalculate();
        return zciis_->fairRate();
    }

    void ZeroCouponInflationSwapHelper::setTermStructure(
            ZeroInflationTermStructure* z) {

        BootstrapHelper<ZeroInflationTermStructure>::setTermStructure(z);

        // set up a new ZCIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;
        Rate K = quote()->value();

        // The effect of the new inflation term structure is
        // felt via the effect on the inflation index
        Handle<ZeroInflationTermStructure> zits(
            boost::shared_ptr<ZeroInflationTermStructure>(z,no_deletion), own);

        boost::shared_ptr<ZeroInflationIndex> new_zii = zii_->clone(zits);

        Real nominal = 1000000.0;   // has to be something but doesn't matter what
        Date start = z->nominalTermStructure()->referenceDate();
        zciis_.reset(new ZeroCouponInflationSwap(
                                ZeroCouponInflationSwap::Payer,
                                nominal, start, maturity_,
                                calendar_, paymentConvention_, dayCounter_, K, // fixed side & fixed rate
                                new_zii, swapObsLag_));
        // Because very simple instrument only takes
        // standard discounting swap engine.
        zciis_->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(z->nominalTermStructure())));
    }


    YearOnYearInflationSwapHelper::YearOnYearInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        const Calendar& calendar,
        BusinessDayConvention paymentConvention,
        const DayCounter& dayCounter,
        const boost::shared_ptr<YoYInflationIndex>& yii)
    : BootstrapHelper<YoYInflationTermStructure>(quote),
    swapObsLag_(swapObsLag), maturity_(maturity),
    calendar_(calendar), paymentConvention_(paymentConvention),
    dayCounter_(dayCounter), yii_(yii) {

        if (yii_->interpolated()) {
            // if interpolated then simple
            earliestDate_ = maturity_ - swapObsLag_;
            latestDate_ = maturity_ - swapObsLag_;
        } else {
            // but if NOT interpolated then the value is valid
            // for every day in an inflation period so you actually
            // get an extended validity, however for curve building
            // just put the first date because using that convention
            // for the base date throughout
            std::pair<Date,Date> limStart = inflationPeriod(maturity_ - swapObsLag_,
                                                            yii_->frequency());
            earliestDate_ = limStart.first;
            latestDate_ = limStart.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // it's interpolation (assuming the start day is spot)
        if (yii_->interpolated()) {
            Period pShift(yii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift > yii_->availabilityLag(),
                       "inconsistency between swap observation of index "
                       << swapObsLag_ <<
                       " index availability " << yii_->availabilityLag() <<
                       " index period " << pShift <<
                       " and index availability " << yii_->availabilityLag() <<
                       " need (obsLag-index period) > availLag");
        }

        registerWith(Settings::instance().evaluationDate());
    }


    Real YearOnYearInflationSwapHelper::impliedQuote() const {
        // what does the term structure imply?
        // in this case just the same value ... trivial case
        // (would not be so for an inflation-linked bond)
        yyiis_->recalculate();
        return yyiis_->fairRate();
    }

    void YearOnYearInflationSwapHelper::setTermStructure(
                YoYInflationTermStructure* y) {

        BootstrapHelper<YoYInflationTermStructure>::setTermStructure(y);

        // set up a new YYIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;

        // The effect of the new inflation term structure is
        // felt via the effect on the inflation index
        Handle<YoYInflationTermStructure> yyts(
            boost::shared_ptr<YoYInflationTermStructure>(y,no_deletion), own);

        boost::shared_ptr<YoYInflationIndex> new_yii = yii_->clone(yyts);

        // always works because tenor is always 1 year so
        // no problem with different days-in-month
        Date from = Settings::instance().evaluationDate();
        Date to = maturity_;
        Schedule fixedSchedule = MakeSchedule().from(from).to(to)
                                    .withTenor(1*Years)
                                    .withConvention(Unadjusted)
                                    .withCalendar(calendar_)// fixed leg gets cal from sched
                                    .backwards();
        Schedule yoySchedule = fixedSchedule;
        Spread spread = 0.0;
        Rate fixedRate = quote()->value();

        Real nominal = 1000000.0;   // has to be something but doesn't matter what
        yyiis_.reset(new YearOnYearInflationSwap(YearOnYearInflationSwap::Payer,
                                                    nominal,
                                                    fixedSchedule,
                                                    fixedRate,
                                                    dayCounter_,
                                                    yoySchedule,
                                                    new_yii,
                                                    swapObsLag_,
                                                    spread,
                                                    dayCounter_,
                                                    calendar_,  // inflation index does not have a calendar
                                                    paymentConvention_));


        // Because very simple instrument only takes
        // standard discounting swap engine.
        yyiis_->setPricingEngine(boost::shared_ptr<PricingEngine>(
                    new DiscountingSwapEngine(y->nominalTermStructure())));
    }

}

