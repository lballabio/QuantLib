/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include "inflationvolatility.hpp"
#include "utilities.hpp"

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/inflation/interpolatedyoyinflationcurve.hpp>

#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>

#include <ql/experimental/inflation/yoycapfloortermpricesurface.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/experimental/inflation/yoyoptionletstripper.hpp>

#include <ql/experimental/inflation/kinterpolatedyoyoptionletvolatilitysurface.hpp>
#include <ql/experimental/inflation/interpolatedyoyoptionletstripper.hpp>

#include <ql/cashflows/capflooredinflationcoupon.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>

#include <iostream>


// local namespace for data
//*************************************************************************
namespace inflation_volatility_test {

    using namespace QuantLib;

    // local data globals
    Handle<YieldTermStructure> nominalEUR;
    Handle<YieldTermStructure> nominalGBP;

    RelinkableHandle<YoYInflationTermStructure> yoyEU;
    RelinkableHandle<YoYInflationTermStructure> yoyUK;

    std::vector<Rate> cStrikesEU;
    std::vector<Rate> fStrikesEU;
    std::vector<Period> cfMaturitiesEU;
    ext::shared_ptr<Matrix> cPriceEU;
    ext::shared_ptr<Matrix> fPriceEU;

    ext::shared_ptr<YoYInflationIndex> yoyIndexUK;
    ext::shared_ptr<YoYInflationIndex> yoyIndexEU;

    std::vector<Rate> cStrikesUK;
    std::vector<Rate> fStrikesUK;
    std::vector<Period> cfMaturitiesUK;
    ext::shared_ptr<Matrix> cPriceUK;
    ext::shared_ptr<Matrix> fPriceUK;

    ext::shared_ptr<InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic> > priceSurfEU;

    void reset() {
        nominalEUR = Handle<YieldTermStructure>();
        nominalGBP = Handle<YieldTermStructure>();
        priceSurfEU.reset();
        yoyEU.linkTo(ext::shared_ptr<YoYInflationTermStructure>());
        yoyUK.linkTo(ext::shared_ptr<YoYInflationTermStructure>());
        yoyIndexUK.reset();
        yoyIndexEU.reset();
        cPriceEU.reset();
        fPriceEU.reset();
        cPriceUK.reset();
        fPriceUK.reset();
        yoyIndexUK.reset();

        cStrikesEU.clear();        
        fStrikesEU.clear();
        cStrikesUK.clear();        
        fStrikesUK.clear();
        cfMaturitiesEU.clear();
        cfMaturitiesUK.clear();
    }

    void setup() {

        // make sure of the evaluation date
        Date eval = Date(Day(23), Month(11), Year(2007));
        Settings::instance().evaluationDate() = eval;

        yoyIndexUK = ext::shared_ptr<YoYInflationIndex>(new YYUKRPIr(true, yoyUK));
        yoyIndexEU = ext::shared_ptr<YoYInflationIndex>(new YYEUHICPr(true, yoyEU));

        // nominal yield curve (interpolated; times assume year parts have 365 days)
        Real timesEUR[] = {0.0109589, 0.0684932, 0.263014, 0.317808, 0.567123, 0.816438,
               1.06575, 1.31507, 1.56438, 2.0137, 3.01918, 4.01644,
               5.01644, 6.01644, 7.01644, 8.01644, 9.02192, 10.0192,
               12.0192, 15.0247, 20.0301, 25.0356, 30.0329, 40.0384,
               50.0466};
        Real ratesEUR[] = {0.0415600, 0.0426840, 0.0470980, 0.0458506, 0.0449550, 0.0439784,
               0.0431887, 0.0426604, 0.0422925, 0.0424591, 0.0421477, 0.0421853,
               0.0424016, 0.0426969, 0.0430804, 0.0435011, 0.0439368, 0.0443825,
               0.0452589, 0.0463389, 0.0472636, 0.0473401, 0.0470629, 0.0461092,
               0.0450794};

        Real timesGBP[] = {0.008219178, 0.010958904, 0.01369863,  0.019178082,  0.073972603,
               0.323287671, 0.57260274,  0.821917808, 1.071232877,  1.320547945,
               1.506849315, 2.002739726, 3.002739726, 4.002739726,  5.005479452,
               6.010958904, 7.008219178, 8.005479452, 9.008219178, 10.00821918,
               12.01369863, 15.0109589,  20.01369863, 25.01917808,  30.02191781,
               40.03287671, 50.03561644, 60.04109589, 70.04931507};
        Real ratesGBP[] = {0.0577363, 0.0582314, 0.0585265, 0.0587165, 0.0596598,
               0.0612506, 0.0589676, 0.0570512, 0.0556147, 0.0546082,
               0.0549492, 0.053801, 0.0529333, 0.0524068, 0.0519712,
               0.0516615, 0.0513711, 0.0510433, 0.0507974, 0.0504833,
               0.0498998, 0.0490464, 0.04768, 0.0464862, 0.045452,
               0.0437699, 0.0425311, 0.0420073, 0.041151};

        std::vector <Real> r;
        std::vector <Date> d;
        Size nTimesEUR = LENGTH(timesEUR);
        Size nTimesGBP = LENGTH(timesGBP);
        for (Size i = 0; i < nTimesEUR; i++) {
            r.push_back(ratesEUR[i]);
            Size ys = (Size)floor(timesEUR[i]);
            Size ds = (Size)((timesEUR[i]-(Real)ys)*365);
            Date dd = eval + Period(ys,Years) + Period(ds,Days);
            d.push_back( dd );
        }

        ext::shared_ptr<InterpolatedZeroCurve<Cubic> >
            euriborTS(new InterpolatedZeroCurve<Cubic>(d, r, Actual365Fixed()));
        Handle<YieldTermStructure> nominalHeur(euriborTS, false);
        nominalEUR = nominalHeur;   // copy to global

        d.clear();
        r.clear();
        for (Size i = 0; i < nTimesGBP; i++) {
            r.push_back(ratesGBP[i]);
            Size ys = (Size)floor(timesGBP[i]);
            Size ds = (Size)((timesGBP[i]-(Real)ys)*365);
            Date dd = eval + Period(ys,Years) + Period(ds,Days);
            d.push_back( dd );
        }

        ext::shared_ptr<InterpolatedZeroCurve<Cubic> >
            gbpLiborTS(new InterpolatedZeroCurve<Cubic>(d, r, Actual365Fixed()));
        Handle<YieldTermStructure> nominalHgbp(gbpLiborTS, false);
        nominalGBP = nominalHgbp;   // copy to global

        // times = years - lag, where the lag is 2 months or 2/12
        // because this data is derived from cap/floor data that
        // is based on a 2 month lag.

        // note that these are NOT swap rates
        // also not that the first value MUST be in the base period
        // i.e. the first rate is for a negative time
        Real yoyEUrates[] = {0.0237951,
             0.0238749, 0.0240334, 0.0241934, 0.0243567, 0.0245323,
             0.0247213, 0.0249348, 0.0251768, 0.0254337, 0.0257258,
             0.0260217, 0.0263006, 0.0265538, 0.0267803, 0.0269378,
             0.0270608, 0.0271363, 0.0272, 0.0272512, 0.0272927,
             0.027317, 0.0273615, 0.0273811, 0.0274063, 0.0274307,
             0.0274625, 0.027527, 0.0275952, 0.0276734, 0.027794};

        d.clear();
        r.clear();
        Date baseDate = TARGET().advance(eval, -2, Months, ModifiedFollowing);
        for (Size i = 0; i < LENGTH(yoyEUrates); i++) {
            Date dd = TARGET().advance(baseDate, i, Years, ModifiedFollowing);
            d.push_back(dd);
            r.push_back(yoyEUrates[i]);
        }

        bool indexIsInterpolated = true;    // actually false for UKRPI but smooth surfaces are
                                            // better for finding intersections etc

        ext::shared_ptr<InterpolatedYoYInflationCurve<Linear> >
            pYTSEU( new InterpolatedYoYInflationCurve<Linear>(
                    eval, TARGET(), Actual365Fixed(), Period(2,Months), Monthly,
                    indexIsInterpolated, d, r) );
        yoyEU.linkTo(pYTSEU);

        // price data
        const Size ncStrikesEU = 6;
        const Size nfStrikesEU = 6;
        const Size ncfMaturitiesEU = 7;
        Real capStrikesEU[ncStrikesEU] = {0.02, 0.025, 0.03, 0.035, 0.04, 0.05};
        Period capMaturitiesEU[ncfMaturitiesEU] = {3*Years, 5*Years, 7*Years,
            10*Years, 15*Years, 20*Years, 30*Years};
        Real capPricesEU[ncStrikesEU][ncfMaturitiesEU] =
            {{116.225, 204.945, 296.285, 434.29, 654.47, 844.775, 1132.33},
                {34.305, 71.575, 114.1, 184.33, 307.595, 421.395, 602.35},
                {6.37, 19.085, 35.635, 66.42, 127.69, 189.685, 296.195},
                {1.325, 5.745, 12.585, 26.945, 58.95, 94.08, 158.985},
                {0.501, 2.37, 5.38, 13.065, 31.91, 53.95, 96.97},
                {0.501, 0.695, 1.47, 4.415, 12.86, 23.75, 46.7}};

        Real floorStrikesEU[nfStrikesEU] = {-0.01, 0.00, 0.005, 0.01, 0.015, 0.02};
        Real floorPricesEU[nfStrikesEU][ncfMaturitiesEU] =
            {{0.501, 0.851, 2.44, 6.645, 16.23, 26.85, 46.365},
                {0.501, 2.236, 5.555, 13.075, 28.46, 44.525, 73.08},
                {1.025, 3.935, 9.095, 19.64, 39.93, 60.375, 96.02},
                {2.465, 7.885, 16.155, 31.6, 59.34, 86.21, 132.045},
                {6.9, 17.92, 32.085, 56.08, 95.95, 132.85, 194.18},
                {23.52, 47.625, 74.085, 114.355, 175.72, 229.565, 316.285}};

        // now load the data into vector and Matrix classes
        cStrikesEU.clear();
        fStrikesEU.clear();
        cfMaturitiesEU.clear();
        for (Real& i : capStrikesEU)
            cStrikesEU.push_back(i);
        for (Real& i : floorStrikesEU)
            fStrikesEU.push_back(i);
        for (auto& i : capMaturitiesEU)
            cfMaturitiesEU.push_back(i);
        ext::shared_ptr<Matrix> tcPriceEU(new Matrix(ncStrikesEU, ncfMaturitiesEU));
        ext::shared_ptr<Matrix> tfPriceEU(new Matrix(nfStrikesEU, ncfMaturitiesEU));
        for(Size i = 0; i < ncStrikesEU; i++) {
            for(Size j = 0; j < ncfMaturitiesEU; j++) {
                (*tcPriceEU)[i][j] = capPricesEU[i][j];
            }
        }
        for(Size i = 0; i < nfStrikesEU; i++) {
            for(Size j = 0; j < ncfMaturitiesEU; j++) {
                (*tfPriceEU)[i][j] = floorPricesEU[i][j];
            }
        }
        cPriceEU = tcPriceEU;   // copy to global
        fPriceEU = tfPriceEU;
    }


    void setupPriceSurface() {

        // construct:
        //  calendar, business day convention, and day counter are
        //  taken from the nominal base give the reference date for
        //  the inflation options (generally 2 or 3 months before
        //  nominal reference date)
        Natural fixingDays = 0;
        Size lag = 3;// must be 3 because we use an interpolated index (EU)
        Period yyLag = Period(lag,Months);
        Rate baseRate = 1; // not really used
        DayCounter dc = Actual365Fixed();
        TARGET cal;
        BusinessDayConvention bdc = ModifiedFollowing;
        const ext::shared_ptr<QuantLib::YieldTermStructure>& pn = nominalEUR.currentLink();
        Handle<QuantLib::YieldTermStructure> n(pn,false);
        ext::shared_ptr<InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic> >
        cfEUprices(new InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic>(
                                       fixingDays,
                                       yyLag, yoyIndexEU, baseRate,
                                       n, dc,
                                       cal,    bdc,
                                       cStrikesEU, fStrikesEU, cfMaturitiesEU,
                                       (*cPriceEU), (*fPriceEU)));

        priceSurfEU = cfEUprices;
    }

}

//***************************************************************************



void InflationVolTest::testYoYPriceSurfaceToVol() {
    BOOST_TEST_MESSAGE("Testing conversion from YoY price surface "
                       "to YoY volatility surface...");

    using namespace inflation_volatility_test;

    SavedSettings backup;

    setup();

    // first get the price surface set up
    setupPriceSurface();

    // caplet pricer, recall that setCapletVolatility(Handle<YoYOptionletVolatilitySurface>)
    // exists ... we'll use it with the -Curve variant of the surface
    // test UNIT DISPLACED pricer
    ext::shared_ptr<YoYOptionletVolatilitySurface> pVS;
    Handle<YoYOptionletVolatilitySurface> hVS(pVS, false); // pVS does NOT own whatever it points to later, hence the handle does not either
    ext::shared_ptr<YoYInflationUnitDisplacedBlackCapFloorEngine>
        yoyPricerUD(new YoYInflationUnitDisplacedBlackCapFloorEngine(yoyIndexEU,hVS,nominalEUR)); //hVS
    // N.B. the vol gets set in the stripper ... else no point!

    // cap stripper
    ext::shared_ptr<YoYOptionletStripper> yoyOptionletStripper(
                             new InterpolatedYoYOptionletStripper<Linear>() );

    // now set up all the variables for the stripping
    Natural settlementDays = 0;
    TARGET cal;
    BusinessDayConvention bdc = ModifiedFollowing;
    DayCounter dc = Actual365Fixed();

    ext::shared_ptr<YoYCapFloorTermPriceSurface> capFloorPrices = priceSurfEU;
    Period lag = priceSurfEU->observationLag();

    Real slope = -0.5; //when you have bad data, i.e. very low/constant
    //prices for short dated extreem strikes
    //then you cannot assume constant caplet vol
    //(else arbitrage)
    // N.B. if this is too extreme then can't
    // get a no-arbitrage solution anyway
    // the way the slope is used means that the slope is
    // proportional to the level so higher slopes at
    // the edges when things are more volatile

    // Actually is doesn't matter what the interpolation is because we only
    // intend to use the K values that correspond to quotes ... for model fitting.
    ext::shared_ptr<KInterpolatedYoYOptionletVolatilitySurface<Linear> > yoySurf(new
                    KInterpolatedYoYOptionletVolatilitySurface<Linear>(settlementDays,
                cal, bdc, dc, lag, capFloorPrices, yoyPricerUD, yoyOptionletStripper,
                                                              slope) );

    // now use it for something ... like stating what the T=const lines look like
    const Real volATyear1[] = {
          0.0128, 0.0093, 0.0083, 0.0073, 0.0064,
          0.0058, 0.0042, 0.0046, 0.0053, 0.0064,
          0.0098
    };
    const Real volATyear3[] = {
          0.0079, 0.0058, 0.0051, 0.0045, 0.0039,
          0.0035, 0.0026, 0.0028, 0.0033, 0.0039,
          0.0060
    };

    Date d = yoySurf->baseDate() + Period(1,Years);
    auto someSlice = yoySurf->Dslice(d);

    Size n = someSlice.first.size();
    Real eps = 0.0001;
    for(Size i = 0; i < n; i++){
        QL_REQUIRE( fabs(someSlice.second[i] - volATyear1[i]) < eps,
                   " could not recover 1yr vol: " << someSlice.second[i]
                   << " vs " << volATyear1[i] );
    }

    d = yoySurf->baseDate() + Period(3,Years);
    auto someOtherSlice = yoySurf->Dslice(d);
    n = someOtherSlice.first.size();
    for(Size i = 0; i < n; i++){
        QL_REQUIRE(fabs(someOtherSlice.second[i]-volATyear3[i]) < eps,
                        "could not recover 3yr vol: "
                        << someOtherSlice.second[i]<< " vs " << volATyear3[i] );
    }

    reset();
}




void InflationVolTest::testYoYPriceSurfaceToATM() {
    BOOST_TEST_MESSAGE("Testing conversion from YoY cap-floor surface "
                       "to YoY inflation term structure...");

    using namespace inflation_volatility_test;

    SavedSettings backup;

    setup();

    setupPriceSurface();

    auto yyATMt = priceSurfEU->atmYoYSwapTimeRates();
    auto yyATMd = priceSurfEU->atmYoYSwapDateRates();

    // Real dy = (Real)lag / 12.0;
    const Real crv[] = {0.024586, 0.0247575, 0.0249396, 0.0252596,
                          0.0258498, 0.0262883, 0.0267915};
    const Real swaps[] = {0.024586, 0.0247575, 0.0249396, 0.0252596,
                          0.0258498, 0.0262883, 0.0267915};
    const Real ayoy[] = {0.0247659, 0.0251437, 0.0255945, 0.0265234,
                           0.0280457, 0.0285534, 0.0295884};
    Real eps = 2e-5;
    for(Size i = 0; i < yyATMt.first.size(); i++) {
        QL_REQUIRE(fabs( yyATMt.second[i] - crv[i] ) < eps,
                   "could not recover cached yoy swap curve "
                   << yyATMt.second[i]<< " vs " << crv[i]);
    }

    for(Size i = 0; i < yyATMd.first.size(); i++) {
        QL_REQUIRE(fabs( priceSurfEU->atmYoYSwapRate(yyATMd.first[i])  - swaps[i] ) < eps,
                   "could not recover yoy swap curve "
                   << priceSurfEU->atmYoYSwapRate(yyATMd.first[i]) << " vs " << swaps[i]);
    }
    for(Size i = 0; i < yyATMd.first.size(); i++) {
        QL_REQUIRE(fabs( priceSurfEU->atmYoYRate(yyATMd.first[i])  - ayoy[i] ) < eps,
                   " could not recover cached yoy curve "
                   << priceSurfEU->atmYoYRate(yyATMd.first[i]) << " vs " << ayoy[i]
                   <<" at "<<yyATMd.first[i]);
    }
    reset();
}


boost::unit_test_framework::test_suite* InflationVolTest::suite() {
    auto* suite = BOOST_TEST_SUITE("yoyOptionletStripper (yoy inflation vol) tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationVolTest::testYoYPriceSurfaceToATM));
    suite->add(QUANTLIB_TEST_CASE(&InflationVolTest::testYoYPriceSurfaceToVol));

    return suite;
}


