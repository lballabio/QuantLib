/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon
 
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

#include "utilities.hpp"
#include "inflationcpiswap.hpp"
#include <ql/types.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/indexes/ibor/gbplibor.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/cpicouponpricer.hpp>
#include <ql/instruments/cpiswap.hpp>
#include <ql/instruments/bonds/cpibond.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#include <iostream>

using std::fabs;

namespace inflation_cpi_swap_test {
    struct Datum {
        Date date;
        Rate rate;
    };

    template <class T, class U, class I>
    std::vector<ext::shared_ptr<BootstrapHelper<T> > > makeHelpers(
        Datum iiData[], Size N,
        const ext::shared_ptr<I> &ii, const Period &observationLag,
        const Calendar &calendar,
        const BusinessDayConvention &bdc,
        const DayCounter &dc,
        const Handle<YieldTermStructure>& discountCurve) {

        std::vector<ext::shared_ptr<BootstrapHelper<T> > > instruments;
        for (Size i=0; i<N; i++) {
            Date maturity = iiData[i].date;
            Handle<Quote> quote(ext::shared_ptr<Quote>(
                                new SimpleQuote(iiData[i].rate/100.0)));
            ext::shared_ptr<BootstrapHelper<T> > anInstrument(new U(quote, observationLag, maturity,
                                                                    calendar, bdc, dc, ii,
                                                                    CPI::AsIndex, discountCurve));
            instruments.push_back(anInstrument);
        }

        return instruments;
    }


    struct CommonVars {
        // common data

        Size length;
        Date startDate;
        Real volatility;

        Frequency frequency;
        std::vector<Real> nominals;
        Calendar calendar;
        BusinessDayConvention convention;
        Natural fixingDays;
        Date evaluationDate;
        Natural settlementDays;
        Date settlement;
        Period observationLag, contractObservationLag;
        CPI::InterpolationType contractObservationInterpolation;
        DayCounter dcZCIIS,dcNominal;
        std::vector<Date> zciisD;
        std::vector<Rate> zciisR;
        ext::shared_ptr<UKRPI> ii;
        Size zciisDataLength;

        RelinkableHandle<YieldTermStructure> nominalTS;
        ext::shared_ptr<ZeroInflationTermStructure> cpiTS;
        RelinkableHandle<ZeroInflationTermStructure> hcpi;

        // cleanup

        SavedSettings backup;
        IndexHistoryCleaner cleaner;

        // setup
        CommonVars()
        : nominals(1,1000000) {

            // option variables
            frequency = Annual;
            // usual setup
            volatility = 0.01;
            length = 7;
            calendar = UnitedKingdom();
            convention = ModifiedFollowing;
            Date today(25, November, 2009);
            evaluationDate = calendar.adjust(today);
            Settings::instance().evaluationDate() = evaluationDate;
            settlementDays = 0;
            fixingDays = 0;
            settlement = calendar.advance(today,settlementDays,Days);
            startDate = settlement;
            dcZCIIS = ActualActual(ActualActual::ISDA);
            dcNominal = ActualActual(ActualActual::ISDA);

            // uk rpi index
            //      fixing data
            Date from(20, July, 2007);
            //Date from(20, July, 2008);
            Date to(20, November, 2009);
            Schedule rpiSchedule = MakeSchedule().from(from).to(to)
            .withTenor(1*Months)
            .withCalendar(UnitedKingdom())
            .withConvention(ModifiedFollowing);
            Real fixData[] = {
                206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
                209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
                216.5, 217.2, 218.4, 217.7, 216,
                212.9, 210.1, 211.4, 211.3, 211.5,
                212.8, 213.4, 213.4, 213.4, 214.4,
                -999.0, -999.0 };

            // link from cpi index to cpi TS
            bool interp = false;// this MUST be false because the observation lag is only 2 months
                                // for ZCIIS; but not for contract if the contract uses a bigger lag.
            ii = ext::make_shared<UKRPI>(interp, hcpi);
            for (Size i=0; i<rpiSchedule.size();i++) {
                ii->addFixing(rpiSchedule[i], fixData[i], true);// force overwrite in case multiple use
            };


            Datum nominalData[] = {
                { Date(26, November, 2009), 0.475 },
                { Date(2, December, 2009), 0.47498 },
                { Date(29, December, 2009), 0.49988 },
                { Date(25, February, 2010), 0.59955 },
                { Date(18, March, 2010), 0.65361 },
                { Date(25, May, 2010), 0.82830 },
                //  { Date(17, June, 2010), 0.7 },  // can't bootstrap with this data point
                { Date(16, September, 2010), 0.78960 },
                { Date(16, December, 2010), 0.93762 },
                { Date(17, March, 2011), 1.12037 },
                { Date(16, June, 2011), 1.31308 },
                { Date(22, September, 2011),1.52011 },
                { Date(25, November, 2011), 1.78399 },
                { Date(26, November, 2012), 2.41170 },
                { Date(25, November, 2013), 2.83935 },
                { Date(25, November, 2014), 3.12888 },
                { Date(25, November, 2015), 3.34298 },
                { Date(25, November, 2016), 3.50632 },
                { Date(27, November, 2017), 3.63666 },
                { Date(26, November, 2018), 3.74723 },
                { Date(25, November, 2019), 3.83988 },
                { Date(25, November, 2021), 4.00508 },
                { Date(25, November, 2024), 4.16042 },
                { Date(26, November, 2029), 4.15577 },
                { Date(27, November, 2034), 4.04933 },
                { Date(25, November, 2039), 3.95217 },
                { Date(25, November, 2049), 3.80932 },
                { Date(25, November, 2059), 3.80849 },
                { Date(25, November, 2069), 3.72677 },
                { Date(27, November, 2079), 3.63082 }
            };

            std::vector<Date> nomD;
            std::vector<Rate> nomR;
            for (auto& i : nominalData) {
                nomD.push_back(i.date);
                nomR.push_back(i.rate / 100.0);
            }
            ext::shared_ptr<YieldTermStructure> nominal =
                ext::make_shared<InterpolatedZeroCurve<Linear>>(nomD,nomR,dcNominal);

            nominalTS.linkTo(nominal);

            // now build the zero inflation curve
            observationLag = Period(2,Months);
            contractObservationLag = Period(3,Months);
            contractObservationInterpolation = CPI::Flat;

            Datum zciisData[] = {
                { Date(25, November, 2010), 3.0495 },
                { Date(25, November, 2011), 2.93 },
                { Date(26, November, 2012), 2.9795 },
                { Date(25, November, 2013), 3.029 },
                { Date(25, November, 2014), 3.1425 },
                { Date(25, November, 2015), 3.211 },
                { Date(25, November, 2016), 3.2675 },
                { Date(25, November, 2017), 3.3625 },
                { Date(25, November, 2018), 3.405 },
                { Date(25, November, 2019), 3.48 },
                { Date(25, November, 2021), 3.576 },
                { Date(25, November, 2024), 3.649 },
                { Date(26, November, 2029), 3.751 },
                { Date(27, November, 2034), 3.77225 },
                { Date(25, November, 2039), 3.77 },
                { Date(25, November, 2049), 3.734 },
                { Date(25, November, 2059), 3.714 },
            };
            zciisDataLength = 17;
            for (Size i = 0; i < zciisDataLength; i++) {
                zciisD.push_back(zciisData[i].date);
                zciisR.push_back(zciisData[i].rate);
            }

            // now build the helpers ...
            std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure> > > helpers =
            makeHelpers<ZeroInflationTermStructure,ZeroCouponInflationSwapHelper,
            ZeroInflationIndex>(zciisData, zciisDataLength, ii,
                                observationLag,
                                calendar, convention, dcZCIIS,
                                Handle<YieldTermStructure>(nominalTS));

            // we can use historical or first ZCIIS for this
            // we know historical is WAY off market-implied, so use market implied flat.
            Rate baseZeroRate = zciisData[0].rate/100.0;
            ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pCPIts(
                                new PiecewiseZeroInflationCurve<Linear>(
                                    evaluationDate, calendar, dcZCIIS, observationLag,
                                    ii->frequency(),ii->interpolated(), baseZeroRate,
                                    helpers));
            pCPIts->recalculate();
            cpiTS = ext::dynamic_pointer_cast<ZeroInflationTermStructure>(pCPIts);


            // make sure that the index has the latest zero inflation term structure
            hcpi.linkTo(pCPIts);
        }
    };

}



void CPISwapTest::consistency() {
    BOOST_TEST_MESSAGE("Checking CPI swap against inflation term structure...");

    using namespace inflation_cpi_swap_test;

    // check inflation leg vs calculation directly from inflation TS
    CommonVars common;

    // ZeroInflationSwap aka CPISwap

    Swap::Type type = Swap::Payer;
    Real nominal = 1000000.0;
    bool subtractInflationNominal = true;
    // float+spread leg
    Spread spread = 0.0;
    DayCounter floatDayCount = Actual365Fixed();
    BusinessDayConvention floatPaymentConvention = ModifiedFollowing;
    Natural fixingDays = 0;
    ext::shared_ptr<IborIndex> floatIndex(new GBPLibor(Period(6,Months),
                                                         common.nominalTS));

    // fixed x inflation leg
    Rate fixedRate = 0.1;//1% would be 0.01
    Real baseCPI = 206.1; // would be 206.13871 if we were interpolating
    DayCounter fixedDayCount = Actual365Fixed();
    BusinessDayConvention fixedPaymentConvention = ModifiedFollowing;
    Calendar fixedPaymentCalendar = UnitedKingdom();
    ext::shared_ptr<ZeroInflationIndex> fixedIndex = common.ii;
    Period contractObservationLag = common.contractObservationLag;
    CPI::InterpolationType observationInterpolation = common.contractObservationInterpolation;

    // set the schedules
    Date startDate(2, October, 2007);
    Date endDate(2, October, 2052);
    Schedule floatSchedule = MakeSchedule().from(startDate).to(endDate)
    .withTenor(Period(6,Months))
    .withCalendar(UnitedKingdom())
    .withConvention(floatPaymentConvention)
    .backwards()
    ;
    Schedule fixedSchedule = MakeSchedule().from(startDate).to(endDate)
    .withTenor(Period(6,Months))
    .withCalendar(UnitedKingdom())
    .withConvention(Unadjusted)
    .backwards()
    ;


    CPISwap zisV(type, nominal, subtractInflationNominal,
                 spread, floatDayCount, floatSchedule,
                 floatPaymentConvention, fixingDays, floatIndex,
                 fixedRate, baseCPI, fixedDayCount, fixedSchedule,
                 fixedPaymentConvention, contractObservationLag,
                 fixedIndex, observationInterpolation);
    Date asofDate = Settings::instance().evaluationDate();

    Real floatFix[] = {0.06255,0.05975,0.0637,0.018425,0.0073438,-1,-1};
    Real cpiFix[] = {211.4,217.2,211.4,213.4,-2,-2};
    for(Size i=0;i<floatSchedule.size(); i++){
        if (floatSchedule[i] < common.evaluationDate) {
            floatIndex->addFixing(floatSchedule[i], floatFix[i],true);//true=overwrite
        }

        ext::shared_ptr<CPICoupon>
        zic = ext::dynamic_pointer_cast<CPICoupon>(zisV.cpiLeg()[i]);
        if (zic != nullptr) {
            if (zic->fixingDate() < (common.evaluationDate - Period(1,Months))) {
                fixedIndex->addFixing(zic->fixingDate(), cpiFix[i],true);
            }
        }
    }

    // simple structure so simple pricing engine - most work done by index
    ext::shared_ptr<DiscountingSwapEngine> dse(new DiscountingSwapEngine(common.nominalTS));
    zisV.setPricingEngine(dse);
    
    // get float+spread & fixed*inflation leg prices separately
    Real testInfLegNPV = 0.0;
    for(Size i=0;i<zisV.leg(0).size(); i++){

        Date zicPayDate = (zisV.leg(0))[i]->date();
        if(zicPayDate > asofDate) {
            testInfLegNPV += (zisV.leg(0))[i]->amount()*common.nominalTS->discount(zicPayDate);
        }

        ext::shared_ptr<CPICoupon>
            zicV = ext::dynamic_pointer_cast<CPICoupon>(zisV.cpiLeg()[i]);
        if (zicV != nullptr) {
            Real diff = fabs( zicV->rate() - (fixedRate*(zicV->indexFixing()/baseCPI)) );
            QL_REQUIRE(diff<1e-8,"failed "<<i<<"th coupon reconstruction as "
                       << (fixedRate*(zicV->indexFixing()/baseCPI)) << " vs rate = "
                       <<zicV->rate() << ", with difference: " << diff);
        }
    }

    Real error = fabs(testInfLegNPV - zisV.legNPV(0));
    QL_REQUIRE(error<1e-5,
               "failed manual inf leg NPV calc vs pricing engine: " <<
               testInfLegNPV << " vs " << zisV.legNPV(0));

    Real diff = fabs(1-zisV.NPV()/4191660.0);
    
    Real max_diff;
    if (Settings::instance().iborCouponSettings().usingAtParCoupons())
        max_diff = 1e-5;
    else
        max_diff = 3e-5;

    QL_REQUIRE(diff<max_diff,
               "failed stored consistency value test, ratio = " << diff);

    // remove circular refernce
    common.hcpi.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());
}


void CPISwapTest::zciisconsistency() {
    BOOST_TEST_MESSAGE("Checking CPI swap against zero-coupon inflation swap...");

    using namespace inflation_cpi_swap_test;

    CommonVars common;

    Swap::Type ztype = Swap::Payer;
    Real  nominal = 1000000.0;
    Date startDate(common.evaluationDate);
    Date endDate(25, November, 2059);
    Calendar cal = UnitedKingdom();
    BusinessDayConvention paymentConvention = ModifiedFollowing;
    DayCounter dummyDC, dc = ActualActual(ActualActual::ISDA);
    Period observationLag(2,Months);

    Rate quote = 0.03714;
    ZeroCouponInflationSwap zciis(ztype, nominal, startDate, endDate, cal, paymentConvention, dc,
                                  quote, common.ii, observationLag, CPI::AsIndex);

    // simple structure so simple pricing engine - most work done by index
    ext::shared_ptr<DiscountingSwapEngine>
    dse(new DiscountingSwapEngine(common.nominalTS));

    zciis.setPricingEngine(dse);
    QL_REQUIRE(fabs(zciis.NPV())<1e-3,"zciis does not reprice to zero");

    std::vector<Date> oneDate = {endDate};
    Schedule schOneDate(oneDate, cal, paymentConvention);

    Swap::Type stype = Swap::Payer;
    Real inflationNominal = nominal;
    Real floatNominal = inflationNominal * std::pow(1.0+quote,50);
    bool subtractInflationNominal = true;
    Real dummySpread=0.0, dummyFixedRate=0.0;
    Natural fixingDays = 0;
    Date baseDate = startDate - observationLag;
    Real baseCPI = common.ii->fixing(baseDate);

    ext::shared_ptr<IborIndex> dummyFloatIndex;

    CPISwap cS(stype, floatNominal, subtractInflationNominal, dummySpread, dummyDC, schOneDate,
               paymentConvention, fixingDays, dummyFloatIndex,
               dummyFixedRate, baseCPI, dummyDC, schOneDate, paymentConvention, observationLag,
               common.ii, CPI::AsIndex, inflationNominal);

    cS.setPricingEngine(dse);
    QL_REQUIRE(fabs(cS.NPV())<1e-3,"CPISwap as ZCIIS does not reprice to zero");

    for (Size i=0; i<2; i++) {
        QL_REQUIRE(fabs(cS.legNPV(i)-zciis.legNPV(i))<1e-3,"zciis leg does not equal CPISwap leg");
    }
    // remove circular refernce
    common.hcpi.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());
}


void CPISwapTest::cpibondconsistency() {
    BOOST_TEST_MESSAGE("Checking CPI swap against CPI bond...");

    using namespace inflation_cpi_swap_test;

    CommonVars common;

    // ZeroInflationSwap aka CPISwap

    Swap::Type type = Swap::Payer;
    Real nominal = 1000000.0;
    bool subtractInflationNominal = true;
    // float+spread leg
    Spread spread = 0.0;
    DayCounter floatDayCount = Actual365Fixed();
    BusinessDayConvention floatPaymentConvention = ModifiedFollowing;
    Natural fixingDays = 0;
    ext::shared_ptr<IborIndex> floatIndex(new GBPLibor(Period(6,Months),
                                                         common.nominalTS));

    // fixed x inflation leg
    Rate fixedRate = 0.1;//1% would be 0.01
    Real baseCPI = 206.1; // would be 206.13871 if we were interpolating
    DayCounter fixedDayCount = Actual365Fixed();
    BusinessDayConvention fixedPaymentConvention = ModifiedFollowing;
    Calendar fixedPaymentCalendar = UnitedKingdom();
    ext::shared_ptr<ZeroInflationIndex> fixedIndex = common.ii;
    Period contractObservationLag = common.contractObservationLag;
    CPI::InterpolationType observationInterpolation = common.contractObservationInterpolation;

    // set the schedules
    Date startDate(2, October, 2007);
    Date endDate(2, October, 2052);
    Schedule floatSchedule = MakeSchedule().from(startDate).to(endDate)
    .withTenor(Period(6,Months))
    .withCalendar(UnitedKingdom())
    .withConvention(floatPaymentConvention)
    .backwards()
    ;
    Schedule fixedSchedule = MakeSchedule().from(startDate).to(endDate)
    .withTenor(Period(6,Months))
    .withCalendar(UnitedKingdom())
    .withConvention(Unadjusted)
    .backwards()
    ;


    CPISwap zisV(type, nominal, subtractInflationNominal,
                 spread, floatDayCount, floatSchedule,
                 floatPaymentConvention, fixingDays, floatIndex,
                 fixedRate, baseCPI, fixedDayCount, fixedSchedule,
                 fixedPaymentConvention, contractObservationLag,
                 fixedIndex, observationInterpolation);

    Real floatFix[] = {0.06255,0.05975,0.0637,0.018425,0.0073438,-1,-1};
    Real cpiFix[] = {211.4,217.2,211.4,213.4,-2,-2};
    for(Size i=0;i<floatSchedule.size(); i++){
        if (floatSchedule[i] < common.evaluationDate) {
            floatIndex->addFixing(floatSchedule[i], floatFix[i],true);//true=overwrite
        }

        ext::shared_ptr<CPICoupon>
        zic = ext::dynamic_pointer_cast<CPICoupon>(zisV.cpiLeg()[i]);
        if (zic != nullptr) {
            if (zic->fixingDate() < (common.evaluationDate - Period(1,Months))) {
                fixedIndex->addFixing(zic->fixingDate(), cpiFix[i],true);
            }
        }
    }


    // simple structure so simple pricing engine - most work done by index
    ext::shared_ptr<DiscountingSwapEngine> dse(new DiscountingSwapEngine(common.nominalTS));
    zisV.setPricingEngine(dse);

    // now do the bond equivalent
    std::vector<Rate> fixedRates(1,fixedRate);
    Natural settlementDays = 1;// cannot be zero!
    bool growthOnly = true;
    CPIBond cpiB(settlementDays, nominal, growthOnly,
                 baseCPI, contractObservationLag, fixedIndex,
                 observationInterpolation, fixedSchedule,
                 fixedRates, fixedDayCount, fixedPaymentConvention);

    ext::shared_ptr<DiscountingBondEngine> dbe(new DiscountingBondEngine(common.nominalTS));
    cpiB.setPricingEngine(dbe);

    QL_REQUIRE(fabs(cpiB.NPV() - zisV.legNPV(0))<1e-5,"cpi bond does not equal equivalent cpi swap leg");
    // remove circular reference
    common.hcpi.linkTo(ext::shared_ptr<ZeroInflationTermStructure>());
}


test_suite* CPISwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("CPISwap tests");

    suite->add(QUANTLIB_TEST_CASE(&CPISwapTest::consistency));
    suite->add(QUANTLIB_TEST_CASE(&CPISwapTest::zciisconsistency));
    suite->add(QUANTLIB_TEST_CASE(&CPISwapTest::cpibondconsistency));

    return suite;
}

