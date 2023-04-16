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


#include "inflationcpicapfloor.hpp"
#include "utilities.hpp"
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
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/cpicouponpricer.hpp>
#include <ql/instruments/cpiswap.hpp>
#include <ql/instruments/bonds/cpibond.hpp>
#include <ql/instruments/cpicapfloor.hpp>
#include <ql/experimental/inflation/cpicapfloortermpricesurface.hpp>
#include <ql/experimental/inflation/cpicapfloorengines.hpp>

#include <iostream>


using namespace QuantLib;
using namespace boost::unit_test_framework;


namespace inflation_cpi_capfloor_test {
    struct Datum {
        Date date;
        Rate rate;
    };

    template <class T, class U, class I>
    std::vector<std::shared_ptr<BootstrapHelper<T> > > makeHelpers(
        Datum iiData[], Size N,
        const std::shared_ptr<I> &ii, const Period &observationLag,
        const Calendar &calendar,
        const BusinessDayConvention &bdc,
        const DayCounter &dc,
        const Handle<YieldTermStructure>& discountCurve) {

        std::vector<std::shared_ptr<BootstrapHelper<T> > > instruments;
        for (Size i=0; i<N; i++) {
            Date maturity = iiData[i].date;
            Handle<Quote> quote(std::shared_ptr<Quote>(
                                new SimpleQuote(iiData[i].rate/100.0)));
            std::shared_ptr<BootstrapHelper<T> > anInstrument(new U(quote, observationLag, maturity,
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
        Rate baseZeroRate;
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
        std::shared_ptr<UKRPI> ii;
        Size zciisDataLength;

        RelinkableHandle<YieldTermStructure> nominalUK;
        RelinkableHandle<ZeroInflationTermStructure> cpiUK;
        RelinkableHandle<ZeroInflationTermStructure> hcpi;

        std::vector<Rate> cStrikesUK;
        std::vector<Rate> fStrikesUK;
        std::vector<Period> cfMaturitiesUK;
        std::shared_ptr<Matrix> cPriceUK;
        std::shared_ptr<Matrix> fPriceUK;

        std::shared_ptr<CPICapFloorTermPriceSurface> cpiCFsurfUK;

        // cleanup

        SavedSettings backup;

        // setup
        CommonVars()
        : nominals(1,1000000) {
            //std::cout <<"CommonVars" << std::endl;
            // option variables
            frequency = Annual;
            // usual setup
            volatility = 0.01;
            length = 7;
            calendar = UnitedKingdom();
            convention = ModifiedFollowing;
            Date today(1, June, 2010);
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
            Date from(1, July, 2007);
            Date to(1, June, 2010);
            Schedule rpiSchedule = MakeSchedule().from(from).to(to)
            .withTenor(1*Months)
            .withCalendar(UnitedKingdom())
            .withConvention(ModifiedFollowing);
            Real fixData[] = {
                206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
                209.8, 211.4, 212.1, 214.0, 215.1, 216.8,   //  2008
                216.5, 217.2, 218.4, 217.7, 216.0, 212.9,
                210.1, 211.4, 211.3, 211.5, 212.8, 213.4,   //  2009
                213.4, 214.4, 215.3, 216.0, 216.6, 218.0,
                217.9, 219.2, 220.7, 222.8, -999, -999,     //  2010
                -999};

            // link from cpi index to cpi TS
            ii = std::make_shared<UKRPI>(hcpi);
            for (Size i=0; i<rpiSchedule.size();i++) {
                ii->addFixing(rpiSchedule[i], fixData[i], true);// force overwrite in case multiple use
            };


            Datum nominalData[] = {
                { Date( 2, June, 2010), 0.499997 },
                { Date( 3, June, 2010), 0.524992 },
                { Date( 8, June, 2010), 0.524974 },
                { Date( 15, June, 2010), 0.549942 },
                { Date( 22, June, 2010), 0.549913 },
                { Date( 1, July, 2010), 0.574864 },
                { Date( 2, August, 2010), 0.624668 },
                { Date( 1, September, 2010), 0.724338 },
                { Date( 16, September, 2010), 0.769461 },
                { Date( 1, December, 2010), 0.997501 },
                //{ Date( 16, December, 2010), 0.838164 },
                { Date( 17, March, 2011), 0.916996 },
                { Date( 16, June, 2011), 0.984339 },
                { Date( 22, September, 2011), 1.06085 },
                { Date( 22, December, 2011), 1.141788 },
                { Date( 1, June, 2012), 1.504426 },
                { Date( 3, June, 2013), 1.92064 },
                { Date( 2, June, 2014), 2.290824 },
                { Date( 1, June, 2015), 2.614394 },
                { Date( 1, June, 2016), 2.887445 },
                { Date( 1, June, 2017), 3.122128 },
                { Date( 1, June, 2018), 3.322511 },
                { Date( 3, June, 2019), 3.483997 },
                { Date( 1, June, 2020), 3.616896 },
                { Date( 1, June, 2022), 3.8281 },
                { Date( 2, June, 2025), 4.0341 },
                { Date( 3, June, 2030), 4.070854 },
                { Date( 1, June, 2035), 4.023202 },
                { Date( 1, June, 2040), 3.954748 },
                { Date( 1, June, 2050), 3.870953 },
                { Date( 1, June, 2060), 3.85298 },
                { Date( 2, June, 2070), 3.757542 },
                { Date( 3, June, 2080), 3.651379 }
            };

            std::vector<Date> nomD;
            std::vector<Rate> nomR;
            for (auto& i : nominalData) {
                nomD.push_back(i.date);
                nomR.push_back(i.rate / 100.0);
            }
            std::shared_ptr<YieldTermStructure> nominalTS =
                std::make_shared<InterpolatedZeroCurve<Linear>>(nomD,nomR,dcNominal);

            nominalUK.linkTo(nominalTS);


            // now build the zero inflation curve
            observationLag = Period(2,Months);
            contractObservationLag = Period(3,Months);
            contractObservationInterpolation = CPI::Flat;

            Datum zciisData[] = {
                { Date(1, June, 2011), 3.087 },
                { Date(1, June, 2012), 3.12 },
                { Date(1, June, 2013), 3.059 },
                { Date(1, June, 2014), 3.11 },
                { Date(1, June, 2015), 3.15 },
                { Date(1, June, 2016), 3.207 },
                { Date(1, June, 2017), 3.253 },
                { Date(1, June, 2018), 3.288 },
                { Date(1, June, 2019), 3.314 },
                { Date(1, June, 2020), 3.401 },
                { Date(1, June, 2022), 3.458 },
                { Date(1, June, 2025), 3.52 },
                { Date(1, June, 2030), 3.655 },
                { Date(1, June, 2035), 3.668 },
                { Date(1, June, 2040), 3.695 },
                { Date(1, June, 2050), 3.634 },
                { Date(1, June, 2060), 3.629 },
            };
            zciisDataLength = 17;
            for (Size i = 0; i < zciisDataLength; i++) {
                zciisD.push_back(zciisData[i].date);
                zciisR.push_back(zciisData[i].rate);
            }

            // now build the helpers ...
            std::vector<std::shared_ptr<BootstrapHelper<ZeroInflationTermStructure> > > helpers =
            makeHelpers<ZeroInflationTermStructure,ZeroCouponInflationSwapHelper,
            ZeroInflationIndex>(zciisData, zciisDataLength, ii,
                                observationLag,
                                calendar, convention, dcZCIIS,
                                Handle<YieldTermStructure>(nominalTS));

            // we can use historical or first ZCIIS for this
            // we know historical is WAY off market-implied, so use market implied flat.
            baseZeroRate = zciisData[0].rate/100.0;
            std::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pCPIts(
                                new PiecewiseZeroInflationCurve<Linear>(
                                    evaluationDate, calendar, dcZCIIS, observationLag,
                                    ii->frequency(), baseZeroRate, helpers));
            pCPIts->recalculate();
            cpiUK.linkTo(pCPIts);

            // make sure that the index has the latest zero inflation term structure
            hcpi.linkTo(pCPIts);

            // cpi CF price surf data
            Period cfMat[] = {3*Years, 5*Years, 7*Years, 10*Years, 15*Years, 20*Years, 30*Years};
            Real cStrike[] = {0.03, 0.04, 0.05, 0.06};
            Real fStrike[] = {-0.01, 0, 0.01, 0.02};
            Size ncStrikes = 4, nfStrikes = 4, ncfMaturities = 7;

            Real cPrice[7][4] = {
                {227.6, 100.27, 38.8, 14.94},
                {345.32, 127.9, 40.59, 14.11},
                {477.95, 170.19, 50.62, 16.88},
                {757.81, 303.95, 107.62, 43.61},
                {1140.73, 481.89, 168.4, 63.65},
                {1537.6, 607.72, 172.27, 54.87},
                {2211.67, 839.24, 184.75, 45.03}};
            Real fPrice[7][4] = {
                {15.62, 28.38, 53.61, 104.6},
                {21.45, 36.73, 66.66, 129.6},
                {24.45, 42.08, 77.04, 152.24},
                {39.25, 63.52, 109.2, 203.44},
                {36.82, 63.62, 116.97, 232.73},
                {39.7, 67.47, 121.79, 238.56},
                {41.48, 73.9, 139.75, 286.75}};

            // now load the data into vector and Matrix classes
            cStrikesUK.clear();
            fStrikesUK.clear();
            cfMaturitiesUK.clear();
            for(Size i = 0; i < ncStrikes; i++) cStrikesUK.push_back(cStrike[i]);
            for(Size i = 0; i < nfStrikes; i++) fStrikesUK.push_back(fStrike[i]);
            for(Size i = 0; i < ncfMaturities; i++) cfMaturitiesUK.push_back(cfMat[i]);
            cPriceUK = std::make_shared<Matrix>(ncStrikes, ncfMaturities);
            fPriceUK = std::make_shared<Matrix>(nfStrikes, ncfMaturities);
            for(Size i = 0; i < ncStrikes; i++) {
                for(Size j = 0; j < ncfMaturities; j++) {
                    (*cPriceUK)[i][j] = cPrice[j][i]/10000.0;
                }
            }
            for(Size i = 0; i < nfStrikes; i++) {
                for(Size j = 0; j < ncfMaturities; j++) {
                    (*fPriceUK)[i][j] = fPrice[j][i]/10000.0;
                }
            }
        }
    };

}



void InflationCPICapFloorTest::cpicapfloorpricesurface() {
    BOOST_TEST_MESSAGE("Checking CPI cap/floor against price surface...");
    
    using namespace inflation_cpi_capfloor_test;
    
    CommonVars common;

    Real nominal = 1.0;
    InterpolatedCPICapFloorTermPriceSurface
    <Bilinear> cpiSurf(nominal,
                       common.baseZeroRate,
                       common.observationLag,
                       common.calendar,
                       common.convention,
                       common.dcZCIIS,
                       common.ii,
                       CPI::Flat,
                       common.nominalUK,
                       common.cStrikesUK,
                       common.fStrikesUK,
                       common.cfMaturitiesUK,
                       *(common.cPriceUK),
                       *(common.fPriceUK));

    // test code - note order of indices
     for (Size i =0; i<common.fStrikesUK.size(); i++){

         Real qK = common.fStrikesUK[i];
         Size nMat = common.cfMaturitiesUK.size();
         for (Size j=0; j<nMat; j++) {
             Period t = common.cfMaturitiesUK[j];
             Real a = (*(common.fPriceUK))[i][j];
             Real b = cpiSurf.floorPrice(t,qK);

             QL_REQUIRE(fabs(a-b)<1e-7,"cannot reproduce cpi floor data from surface: "
                        << a << " vs constructed = " << b);
         }

     }

    for (Size i =0; i<common.cStrikesUK.size(); i++){

        Real qK = common.cStrikesUK[i];
        Size nMat = common.cfMaturitiesUK.size();
        for (Size j=0; j<nMat; j++) {
            Period t = common.cfMaturitiesUK[j];
            Real a = (*(common.cPriceUK))[i][j];
            Real b = cpiSurf.capPrice(t,qK);

            QL_REQUIRE(fabs(a-b)<1e-7,"cannot reproduce cpi cap data from surface: "
                       << a << " vs constructed = " << b);
        }
    }

    // Test the price method also i.e. does it pick out the correct premium?
    // Look up premium from surface at 3 years and strike of 1%
    // Expect, as 1% < ATM, to get back floor premium at 1% i.e. 53.61 bps
    Real premium = cpiSurf.price(3 * Years, 0.01);
    Real expPremium = (*common.fPriceUK)[2][0];
    if (fabs(premium - expPremium) > 1e-12) {
        BOOST_ERROR("The requested premium, " << premium
            << ", does not equal the expected premium, " << expPremium << ".");
    }

    // remove circular refernce
    common.hcpi.linkTo(std::shared_ptr<ZeroInflationTermStructure>());
}


void InflationCPICapFloorTest::cpicapfloorpricer() {
    BOOST_TEST_MESSAGE("Checking CPI cap/floor pricer...");
    
    using namespace inflation_cpi_capfloor_test;

    CommonVars common;
    Real nominal = 1.0;
    std::shared_ptr<CPICapFloorTermPriceSurface> cpiCFpriceSurf(new InterpolatedCPICapFloorTermPriceSurface
                                                    <Bilinear>(nominal,
                                                               common.baseZeroRate,
                                                               common.observationLag,
                                                               common.calendar,
                                                               common.convention,
                                                               common.dcZCIIS,
                                                               common.ii,
                                                               CPI::Flat,
                                                               common.nominalUK,
                                                               common.cStrikesUK,
                                                               common.fStrikesUK,
                                                               common.cfMaturitiesUK,
                                                               *(common.cPriceUK),
                                                               *(common.fPriceUK)));

    common.cpiCFsurfUK = cpiCFpriceSurf;

    // interpolation pricer first
    // N.B. no new instrument required but we do need a new pricer

    Date startDate = Settings::instance().evaluationDate();
    Date maturity(startDate + Period(3,Years));
    Calendar fixCalendar = UnitedKingdom(), payCalendar = UnitedKingdom();
    BusinessDayConvention fixConvention(Unadjusted), payConvention(ModifiedFollowing);
    Rate strike(0.03);
    Real baseCPI = common.ii->fixing(fixCalendar.adjust(startDate-common.observationLag,fixConvention));
    CPI::InterpolationType observationInterpolation = CPI::AsIndex;
    CPICapFloor aCap(Option::Call,
                     nominal,
                     startDate,   // start date of contract (only)
                     baseCPI,
                     maturity,    // this is pre-adjustment!
                     fixCalendar,
                     fixConvention,
                     payCalendar,
                     payConvention,
                     strike,
                     common.ii,
                     common.observationLag,
                     observationInterpolation);

    Handle<CPICapFloorTermPriceSurface> cpiCFsurfUKh(common.cpiCFsurfUK);
    std::shared_ptr<PricingEngine>engine(new InterpolatingCPICapFloorEngine(cpiCFsurfUKh));

    aCap.setPricingEngine(engine);

    // We should get back the cap premium at strike 0.03 i.e. 227.6 bps
    Real cached = (*common.cPriceUK)[0][0];

    QL_REQUIRE(fabs(cached - aCap.NPV())<1e-10,"InterpolatingCPICapFloorEngine does not reproduce cached price: "
               << cached << " vs " << aCap.NPV());

    // remove circular refernce
    common.hcpi.linkTo(std::shared_ptr<ZeroInflationTermStructure>());
}




test_suite* InflationCPICapFloorTest::suite() {
    auto* suite = BOOST_TEST_SUITE("CPIswaption tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationCPICapFloorTest::cpicapfloorpricesurface));
    suite->add(QUANTLIB_TEST_CASE(&InflationCPICapFloorTest::cpicapfloorpricer));

    return suite;
}

