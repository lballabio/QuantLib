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

#include "inflationvol.hpp"
#include "utilities.hpp"

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>

#include <ql/experimental/inflation/inflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcouponpricer.hpp>

#include <ql/experimental/inflation/yoycapfloortermpricesurface.hpp>
#include <ql/experimental/inflation/inflationcapfloorengines.hpp>
#include <ql/experimental/inflation/yoyoptionletstripper.hpp>

#include <ql/experimental/inflation/kinterpolatedyoyoptionletvolatilitysurface.hpp>
#include <ql/experimental/inflation/interpolatedyoyoptionletstripper.hpp>

#include <ql/experimental/inflation/capflooredinflationcoupon.hpp>
#include <ql/indexes/inflation/euhicp.hpp>

#ifndef LENGTH
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))
#endif



// anonymous local namespace for data
//****************************************************************************************
namespace {
    using namespace std;
    using namespace boost;
    using namespace QuantLib;

    void no_deletion(void*) {}

    // local data globals
    Handle<YieldTermStructure> nominalEUR;
    Handle<YieldTermStructure> nominalGBP;

    Handle<YoYInflationTermStructure> yoyEU;
    Handle<YoYInflationTermStructure> yoyUK;

    vector<Rate> cStrikesEU;
    vector<Rate> fStrikesEU;
    vector<Time> cfMaturitiesEU;
    shared_ptr<Matrix> cPriceEU;
    shared_ptr<Matrix> fPriceEU;

    vector<Rate> cStrikesFR;
    vector<Rate> fStrikesFR;
    vector<Time> cfMaturitiesFR;
    shared_ptr<Matrix> cPriceFR;
    shared_ptr<Matrix> fPriceFR;

    vector<Rate> cStrikesUK;
    vector<Rate> fStrikesUK;
    vector<Time> cfMaturitiesUK;
    shared_ptr<Matrix> cPriceUK;
    shared_ptr<Matrix> fPriceUK;

    shared_ptr<InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic> > priceSurfEU;


    void setup() {

        // make sure of the evaluation date
        Date eval = Date(23, November, 2007);
        Settings::instance().evaluationDate() = eval;
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

        vector <Real> r;
        vector <Date> d;
        Size nTimesEUR = LENGTH(timesEUR);
        Size nTimesGBP = LENGTH(timesGBP);
        for (Size i = 0; i < nTimesEUR; i++) {
            r.push_back(ratesEUR[i]);
            Size ys = (Size)floor(timesEUR[i]);
            Size ds = (Size)((timesEUR[i]-(Real)ys)*365);
            Date dd = eval + Period(ys,Years) + Period(ds,Days);
            d.push_back( dd );
        }

        shared_ptr<InterpolatedZeroCurve<Cubic> >
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

        shared_ptr<InterpolatedZeroCurve<Cubic> >
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

        shared_ptr<InterpolatedYoYInflationCurve<Linear> >
            pYTSEU( new InterpolatedYoYInflationCurve<Linear>(
                    baseDate, TARGET(), Actual365Fixed(), Period(2,Months), Monthly, nominalGBP, d, r) );
        Handle<YoYInflationTermStructure> hYTSEU(pYTSEU, false);
        yoyEU = hYTSEU;

        // price data
        const Size ncStrikesEU = 6;
        const Size nfStrikesEU = 6;
        const Size ncfMaturitiesEU = 7;
        Real capStrikesEU[ncStrikesEU] = {0.02, 0.025, 0.03, 0.035, 0.04, 0.05};
        Real capMaturitiesEU[ncfMaturitiesEU] = {3, 5, 7, 10, 15, 20, 30};
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
        for(Size i = 0; i < ncStrikesEU; i++) cStrikesEU.push_back(capStrikesEU[i]);
        for(Size i = 0; i < nfStrikesEU; i++) fStrikesEU.push_back(floorStrikesEU[i]);
        for(Size i = 0; i < ncfMaturitiesEU; i++) cfMaturitiesEU.push_back(capMaturitiesEU[i]);
        shared_ptr<Matrix> tcPriceEU(new Matrix(ncStrikesEU, ncfMaturitiesEU));
        shared_ptr<Matrix> tfPriceEU(new Matrix(nfStrikesEU, ncfMaturitiesEU));
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
        // calendar, business day convention, and day counter are
        // taken from the nominal base give the reference date for
        // the inflation options (generally 2 or 3 months before
        // nominal reference date)
        Natural fixingDays = 0;
        Size lag = 2;
        Period yyLag = Period(lag,Months);
        Frequency frequency = Monthly;
        Rate baseRate = 1; // not really used
        DayCounter dc = Actual365Fixed();
        TARGET cal;
        BusinessDayConvention bdc = ModifiedFollowing;
        boost::shared_ptr<QuantLib::YieldTermStructure> pn =
            nominalEUR.currentLink();
        Handle<QuantLib::YieldTermStructure> n(pn,false);
        boost::shared_ptr<InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic> >
        cfEUprices(new InterpolatedYoYCapFloorTermPriceSurface<Bicubic,Cubic>(
                                       fixingDays,
                                       yyLag, frequency, baseRate,
                                       n, dc,
                                       cal,    bdc,
                                       cStrikesEU, fStrikesEU, cfMaturitiesEU,
                                       (*cPriceEU), (*fPriceEU)));

        priceSurfEU = cfEUprices;
    }

}
//****************************************************************************************



void InflationVolTest::testYoYPriceSurfaceToVol() {
    BOOST_MESSAGE("Testing conversion from YoY price surface "
                  "to YoY volatility surface...");

    SavedSettings backup;

    setup();

    // first get the price surface set up
    setupPriceSurface();

    // caplet pricer, recall that setCapletVolatility(Handle<YoYOptionletVolatilitySurface>)
    // exists ... we'll use it with the -Curve variant of the surface
    // test UNIT DISPLACED pricer
    boost::shared_ptr<YoYOptionletVolatilitySurface> pVS;
    Handle<YoYOptionletVolatilitySurface> hVS(pVS, false); // pVS does NOT own whatever it points to later, hence the handle does not either
    boost::shared_ptr<UnitDisplacedBlackYoYInflationCapFloorEngine>
        yoyPricerUD(new UnitDisplacedBlackYoYInflationCapFloorEngine(nominalEUR,hVS)); //hVS
    // N.B. the vol gets set in the stripper ... else no point!

    // cap stripper
    boost::shared_ptr<YoYOptionletStripper> yoyOptionletStripper(
                             new InterpolatedYoYOptionletStripper<Linear>() );

    // now set up all the variables for the stripping
    Natural settlementDays = 0;
    TARGET cal;
    BusinessDayConvention bdc = ModifiedFollowing;
    DayCounter dc = Actual365Fixed();

    boost::shared_ptr<YoYCapFloorTermPriceSurface> capFloorPrices = priceSurfEU;
    Period lag = priceSurfEU->lag();

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
    boost::shared_ptr<KInterpolatedYoYOptionletVolatilitySurface<Linear> > yoySurf(new
                    KInterpolatedYoYOptionletVolatilitySurface<Linear>(settlementDays,
                cal, bdc, dc, lag, capFloorPrices, yoyPricerUD, yoyOptionletStripper,
                                                              slope) );

    // now use it for something ... like stating what the T lines look like
    const Real volATyear1[] = {
          0.0137987, 0.0101426, 0.00907921, 0.00808054, 0.00715028,
          0.00657454, 0.00378432, 0.00457774, 0.00544686, 0.00661224, 0.0102096
    };
    const Real volATyear3[] = {
          0.00702216, 0.00516338, 0.00462127, 0.00411196, 0.00363747,
          0.00334323, 0.00192446, 0.00232778, 0.0027707, 0.00336424, 0.00519297
    };

    Date d = yoySurf->baseDate() + Period(1,Years);
    pair<vector<Rate>, vector<Volatility> > someSlice;
    someSlice = yoySurf->Dslice(d);

    Size n = someSlice.first.size();
    Real eps = 0.00001;
    for(Size i = 0; i < n; i++){
    QL_REQUIRE( abs(someSlice.second[i] - volATyear1[i]) < eps,"could not recover 1yr vol: "
            << someSlice.second[i]<< " vs " << volATyear1[i]
          );
    }

    d = yoySurf->baseDate() + Period(3,Years);
    pair<vector<Rate>, vector<Volatility> >
        someOtherSlice = yoySurf->Dslice(d);
    n = someOtherSlice.first.size();
    for(Size i = 0; i < n; i++){
    QL_REQUIRE( abs(someOtherSlice.second[i] - volATyear3[i]) < eps,"could not recover 3yr vol: "
            << someOtherSlice.second[i]<< " vs " << volATyear3[i]
          );
    }
}



//------------------------------------------------------------------------
void InflationVolTest::testCappedFlooredYoYInflationCoupon() {
//----------------------------------------------------------------------
    BOOST_MESSAGE("Testing capped-floored YoY inflation coupon...");

    SavedSettings backup;

    setup();
    Date eval = Settings::instance().evaluationDate();
    BusinessDayConvention bdc = ModifiedFollowing;
    TARGET cal;

    Date pay = cal.advance(eval, Period(29,Years),  bdc);
    Date start = cal.advance(eval, Period(28,Years),  bdc);
    Date end = pay;
    Period lag = Period(2,Months);
    Natural fixingDays = 0;

    boost::shared_ptr<YYEUHICP> yoyIndexEU(new YYEUHICP(Monthly, false, false, yoyEU));
    YoYInflationCoupon yoyCouponEU(pay, 1000000.0, start, end, lag,
                     fixingDays, yoyIndexEU);

    // 0.1% constant vol (low for 29 years)
    // for BLACK version set the validity limits to [0%, 300%], i.e. no negative strike possible
    boost::shared_ptr<ConstantYoYOptionletVolatility> yoyVolBLACK(new ConstantYoYOptionletVolatility(0.001,lag,0.0,3.0));
    Handle<YoYOptionletVolatilitySurface> yoyVolBLACKh(yoyVolBLACK,false);

    // pricers
    boost::shared_ptr<BlackYoYInflationCouponPricer>
    yoyPricer(new BlackYoYInflationCouponPricer(yoyVolBLACKh));
    boost::shared_ptr<UnitDisplacedBlackYoYInflationCouponPricer> //
    yoyPricerUD(new UnitDisplacedBlackYoYInflationCouponPricer(yoyVolBLACKh));
    boost::shared_ptr<BachelierYoYInflationCouponPricer>
    yoyPricerBAC(new BachelierYoYInflationCouponPricer(yoyVolBLACKh));

    yoyCouponEU.setPricer(yoyPricer);

    // caplet
    // we need paysWithin = false for caplet; else we get a capped coupon not a caplet
    // difference is whether we look from a coupon or instrument perspective
    bool paysWithin = false;
    CappedFlooredYoYInflationCoupon yoyCapletEU(pay, 1000000.0, start, end, lag,
                          fixingDays, yoyIndexEU,
                          1.0, 0.0, Actual365Fixed(), ModifiedFollowing, cal,
                          paysWithin, 0.0242165);    // this is ATM at 3 yrs

    //cout << "--------------------------------------------------------------------------"<<endl;
    yoyCapletEU.setPricer(yoyPricer);
    //cout << "BLACK rate of caplet really is = " <<  yoyCapletEU.rate() << endl;
    //cout << "BLACK price of caplet really is = " <<  yoyCapletEU.amount() << endl;
    Real eps = 1e-7;
    Real eps2 = 1e-2; // big because just checking printed values
    QL_REQUIRE(abs(yoyCapletEU.rate()-0.00345498)<eps,"Black cap rate wrong");
    QL_REQUIRE(abs(yoyCapletEU.amount()-3473.91)<eps2,"Black cap amount wrong");
    yoyCapletEU.setPricer(yoyPricerUD);
    //cout << "BLACK-UD rate of caplet really is = " <<  yoyCapletEU.rate() << endl;
    //cout << "BLACK-UD price of caplet really is = " <<  yoyCapletEU.amount() << endl;
    QL_REQUIRE(abs(yoyCapletEU.rate()-0.00434459)<eps,"Unit displacement Black cap rate wrong");
    QL_REQUIRE(abs(yoyCapletEU.amount()-4368.4)<eps2,"Unit displacement Black cap amount wrong");
    yoyCapletEU.setPricer(yoyPricerBAC);
    //cout << "BACHELIER rate of caplet really is = " <<  yoyCapletEU.rate() << endl;
    //cout << "BACHELIER price of caplet really is = " <<  yoyCapletEU.amount() << endl;
    QL_REQUIRE(abs(yoyCapletEU.rate()-0.00429915)<eps,"Bachelier cap rate wrong");
    QL_REQUIRE(abs(yoyCapletEU.amount()-4322.7)<eps2,"Bachelier cap amount wrong");
    //cout << "--------------------------------------------------------------------------"<<endl;

    // repeat the exercise using instrument/engine access
    YoYInflationLeg onlyOne;
    boost::shared_ptr<CappedFlooredYoYInflationCoupon> theCoupon(
                             new CappedFlooredYoYInflationCoupon(yoyCapletEU) );
    onlyOne.push_back(theCoupon);
    std::vector<Rate> strikes(1,0.0242165);
    YoYInflationCap instVersion(onlyOne, strikes); // make inst from (short!) leg

    boost::shared_ptr<BlackYoYInflationCapFloorEngine> myEngine(
                            new BlackYoYInflationCapFloorEngine(nominalEUR, yoyVolBLACKh) );
    instVersion.setPricingEngine(myEngine);
    yoyCapletEU.setPricer(yoyPricer);
    Real am = yoyCapletEU.amount() * nominalEUR->discount( yoyCapletEU.date() );
    //cout << instVersion.NPV() << " was NPV (Black), versus " << am << endl;
    QL_REQUIRE(abs(instVersion.NPV()-am)<eps,
         "Black NPV's differ between engine and pricer");

    boost::shared_ptr<UnitDisplacedBlackYoYInflationCapFloorEngine> myEngineUD(
                                   new UnitDisplacedBlackYoYInflationCapFloorEngine(nominalEUR, yoyVolBLACKh) );
    instVersion.setPricingEngine(myEngineUD);
    yoyCapletEU.setPricer(yoyPricerUD);
    am = yoyCapletEU.amount() * nominalEUR->discount( yoyCapletEU.date() );
    //cout << instVersion.NPV() << " was NPV (DD1), versus " << am << endl;
    QL_REQUIRE(abs(instVersion.NPV()-am)<eps,
         "Unit Displaced Black NPV's differ between engine and pricer");

    boost::shared_ptr<BachelierYoYInflationCapFloorEngine> myEngineBAC(
                               new BachelierYoYInflationCapFloorEngine(nominalEUR, yoyVolBLACKh) );
    instVersion.setPricingEngine(myEngineBAC);
    yoyCapletEU.setPricer(yoyPricerBAC);
    am = yoyCapletEU.amount() * nominalEUR->discount( yoyCapletEU.date() );
    //cout << instVersion.NPV() << " was NPV (Bachelier), versus " << am << endl;
    QL_REQUIRE(abs(instVersion.NPV()-am)<eps,
         "Bachelier NPV's differ between engine and pricer");

    // now repeat the exercise using MakeYoYInflationCapFloor to do the work
    // which will be different because the Make... creates caplets for
    // every year whereas we're been doing just payment date Y29.

    Date pay1 = cal.advance(eval, Period(1,Years),  bdc);
    Date start1 = cal.advance(eval, Period(0,Years),  bdc);
    Date end1 = pay1;
    CappedFlooredYoYInflationCoupon yoyCapletEU1(pay1, 1000000.0, start1, end1, lag,
                           fixingDays, yoyIndexEU,
                           1.0, 0.0, Actual365Fixed(), ModifiedFollowing, cal,
                           paysWithin, 0.0242165);
    yoyCapletEU1.setPricer(yoyPricerUD);
    am = yoyCapletEU1.amount() * nominalEUR->discount( yoyCapletEU1.date() );
    //cout << "(1) BLACK-UD NPV of caplet really is = " << am << endl;
    Size n = 1;
    YoYInflationCapFloor made1 = MakeYoYInflationCapFloor(YoYInflationCapFloor::Cap, lag,
                                fixingDays, yoyIndexEU, 0.0242165, n)
    .withNominal(1000000.0);
    made1.setPricingEngine(myEngineUD);
    //cout << "(1) made cap version NPV = " << made1.NPV() << endl;

    YoYInflationCoupon yoyCouponEU1(pay1, 1000000.0, start1, end1, lag,
                      fixingDays, yoyIndexEU,
                      1.0, 0.0, Actual365Fixed(), ModifiedFollowing);
    YoYInflationLeg onlyOne1;
    boost::shared_ptr<YoYInflationCoupon> theCoupon1 = boost::shared_ptr<YoYInflationCoupon>(
                    new YoYInflationCoupon(yoyCouponEU1) );
    onlyOne1.push_back(theCoupon1);
    std::vector<Rate> strikes1(1,0.0242165);
    YoYInflationCap instVersion1(onlyOne1, strikes1); // make inst from (short!) leg
    instVersion1.setPricingEngine(myEngineUD);
    //cout << "(1) long const cap version NPV = " << instVersion1.NPV() << endl;
    Real eps3 = 3.0; //bps, day count conventions
    QL_REQUIRE(abs(made1.NPV()-instVersion1.NPV()) < eps3, "Make-version and by-hand versions "
         << "have different Cap prices: " << made1.NPV() << " versus "<<instVersion1.NPV());

    // floorlet

    paysWithin = false;
    CappedFlooredYoYInflationCoupon yoyFloorletEU(pay, 1000000.0, start, end, lag,
                            fixingDays, yoyIndexEU,
                            1.0, 0.0, Actual365Fixed(), ModifiedFollowing, cal,
                            paysWithin, Null<Rate>(), 0.028);

    //cout << "--------------------------------------------------------------------------"<<endl;
    yoyFloorletEU.setPricer(yoyPricer);
    //cout << "BLACK rate of Floorlet really is = " <<  yoyFloorletEU.rate() << endl;
    //cout << "BLACK price of Floorlet really is = " <<  yoyFloorletEU.amount() << endl;
    QL_REQUIRE(abs(yoyFloorletEU.rate()-0.00032926)<eps,"Black YY cap rate wrong");
    QL_REQUIRE(abs(yoyFloorletEU.amount()-331.064)<eps2,"Black YY cap amount wrong");
    yoyFloorletEU.setPricer(yoyPricerUD);
    //cout << "BLACK-UD rate of Floorlet really is = " <<  yoyFloorletEU.rate() << endl;
    //cout << "BLACK-UD price of Floorlet really is = " <<  yoyFloorletEU.amount() << endl;
    QL_REQUIRE(abs(yoyFloorletEU.rate()-0.0023709)<eps,"Unit displacement Black YY cap rate wrong");
    QL_REQUIRE(abs(yoyFloorletEU.amount()-2383.89)<eps2,"Unit displacement Black YY cap amount wrong");
    yoyFloorletEU.setPricer(yoyPricerBAC);
    //cout << "BACHELIER rate of Floorlet really is = " <<  yoyFloorletEU.rate() << endl;
    //cout << "BACHELIER price of Floorlet really is = " <<  yoyFloorletEU.amount() << endl;
    QL_REQUIRE(abs(yoyFloorletEU.rate()-0.00231135)<eps,"Bachelier YY cap rate wrong");
    QL_REQUIRE(abs(yoyFloorletEU.amount()-2324.02)<eps2,"Bachelier YY cap amount wrong");
    //cout << "--------------------------------------------------------------------------"<<endl;
}



void InflationVolTest::testYoYPriceSurfaceToATM() {
    BOOST_MESSAGE("Testing conversion from YoY cap-floor surface "
                  "to YoY inflation term structure...");

    SavedSettings backup;

    setup();

    setupPriceSurface();

    pair<vector<Time>, vector<Rate> > yyATM = priceSurfEU->atmYoYSwapRates();
    Size lag = 2;
    Real dy = (Real)lag / 12.0;
    const Real crv[] = {0.024586, 0.0247575, 0.0249396, 0.0252596,
                          0.0258498, 0.0262883, 0.0267915};
    const Real swaps[] = {0.024586, 0.0247575, 0.0249396, 0.0252596,
                          0.0258498, 0.0262883, 0.0267915};
    const Real ayoy[] = {0.0247659, 0.0251437, 0.0255945, 0.0265234,
                           0.0280457, 0.0285534, 0.0295884};
    Real eps = 0.000001;
    for(Size i = 0; i < yyATM.first.size(); i++) {
        QL_REQUIRE(abs( yyATM.second[i] - crv[i] ) < eps,
                   "could not recover yoy swap curve "
                   << yyATM.second[i]<< " vs " << crv[i]);
    }
    for(Size i = 0; i < yyATM.first.size(); i++) {
        QL_REQUIRE(abs( priceSurfEU->atmYoYSwapRate(yyATM.first[i])  - swaps[i] ) < eps,
                   "could not recover yoy swap curve "
                   << priceSurfEU->atmYoYSwapRate(yyATM.first[i]) << " vs " << swaps[i]);
    }
    for(Size i = 0; i < yyATM.first.size(); i++) {
        QL_REQUIRE(abs( priceSurfEU->atmYoYRate(yyATM.first[i] - dy)  - ayoy[i] ) < eps,
                   "could not recover yoy curve "
                   << priceSurfEU->atmYoYRate(yyATM.first[i] - dy) << " vs " << ayoy[i]);
    }
}


boost::unit_test_framework::test_suite* InflationVolTest::suite() {
    boost::unit_test_framework::test_suite* suite
        = BOOST_TEST_SUITE("yoyOptionletStripper (yoy inflation vol) tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationVolTest::testYoYPriceSurfaceToATM));
    suite->add(QUANTLIB_TEST_CASE(&InflationVolTest::testYoYPriceSurfaceToVol));
    suite->add(QUANTLIB_TEST_CASE(
                       &InflationVolTest::testCappedFlooredYoYInflationCoupon));

    return suite;
}


