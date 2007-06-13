/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 Giorgio Facchinetti

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

#include "cms.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube1.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CmsTest)


//******************************************************************************************//
//******************************************************************************************//

// General settings
Date referenceDate_, today_, settlement_;
Calendar calendar_;

// Volatility Stuctures
std::vector<Handle<SwaptionVolatilityStructure> > swaptionVolatilityStructures_;
Handle<SwaptionVolatilityStructure> atmVol_;
Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCube1_;
Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCube2_;
Handle<SwaptionVolatilityStructure> swaptionVolatilityCubeBySabr_;

std::vector<Period> atmOptionTenors_, optionTenors_;
std::vector<Period> atmSwapTenors_, swapTenors_;
std::vector<Spread> strikeSpreads_;

Matrix atmVolMatrix_, volSpreadsMatrix_;
std::vector<std::vector<Handle<Quote> > > volSpreads_;

DayCounter dayCounter_;
BusinessDayConvention optionBDC_;
Natural swapSettlementDays_;
bool vegaWeightedSmileFit_;

// Cms Schedules conventions
Frequency fixedCmsFrequency_, floatingCmsFrequency_;
BusinessDayConvention fixedCmsConvention_, floatingCmsConvention_;

// Cms valuation
std::vector<GFunctionFactory::ModelOfYieldCurve> modelOfYieldCurves_;
Rate infiniteCap_, infiniteFloor_;
Real gearing_;
Spread spread_;
Date startDate_;
Date paymentDate_;
Date endDate_;
Natural settlementDays_;
DayCounter fixedCmsDayCount_;

// Term Structure
RelinkableHandle<YieldTermStructure> termStructure_;

// indices and index conventions
Frequency fixedLegFrequency_;
BusinessDayConvention fixedLegConvention_;
DayCounter fixedLegDayCounter_;
boost::shared_ptr<IborIndex> iborIndex_;
boost::shared_ptr<SwapIndex> swapIndexBase_;
Time shortTenor_;
boost::shared_ptr<IborIndex> iborIndexShortTenor_;
boost::shared_ptr<SwapIndex> index_;

//test parameters
Real rateTolerance_;
Real priceTolerance_;


//******************************************************************************************//
//******************************************************************************************//

void setup() {

    //General Settings
    calendar_ = TARGET();
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_, 0.05, Actual365Fixed()));
    referenceDate_ = termStructure_->referenceDate();


    // ATM swaptionvol matrix
    optionBDC_ = Following;

    atmOptionTenors_ = std::vector<Period>();
    atmOptionTenors_.push_back(Period(1, Months));
    atmOptionTenors_.push_back(Period(6, Months));
    atmOptionTenors_.push_back(Period(1, Years));
    atmOptionTenors_.push_back(Period(5, Years));
    atmOptionTenors_.push_back(Period(10, Years));
    atmOptionTenors_.push_back(Period(30, Years));

    atmSwapTenors_ = std::vector<Period>();
    atmSwapTenors_.push_back(Period(1, Years));
    atmSwapTenors_.push_back(Period(5, Years));
    atmSwapTenors_.push_back(Period(10, Years));
    atmSwapTenors_.push_back(Period(30, Years));

    atmVolMatrix_ = Matrix(atmOptionTenors_.size(), atmSwapTenors_.size());
    atmVolMatrix_[0][0]=0.1300; atmVolMatrix_[0][1]=0.1560; atmVolMatrix_[0][2]=0.1390; atmVolMatrix_[0][3]=0.1220;
    atmVolMatrix_[1][0]=0.1440; atmVolMatrix_[1][1]=0.1580; atmVolMatrix_[1][2]=0.1460; atmVolMatrix_[1][3]=0.1260;
    atmVolMatrix_[2][0]=0.1600; atmVolMatrix_[2][1]=0.1590; atmVolMatrix_[2][2]=0.1470; atmVolMatrix_[2][3]=0.1290;
    atmVolMatrix_[3][0]=0.1640; atmVolMatrix_[3][1]=0.1470; atmVolMatrix_[3][2]=0.1370; atmVolMatrix_[3][3]=0.1220;
    atmVolMatrix_[4][0]=0.1400; atmVolMatrix_[4][1]=0.1300; atmVolMatrix_[4][2]=0.1250; atmVolMatrix_[4][3]=0.1100;
    atmVolMatrix_[5][0]=0.1130; atmVolMatrix_[5][1]=0.1090; atmVolMatrix_[5][2]=0.1070; atmVolMatrix_[5][3]=0.0930;

    Size nRowsAtmVols = atmVolMatrix_.rows();
    Size nColsAtmVols = atmVolMatrix_.columns();


    //swaptionvolcube
    optionTenors_ = std::vector<Period>();
    optionTenors_.push_back(Period(1, Years));
    optionTenors_.push_back(Period(10, Years));
    optionTenors_.push_back(Period(30, Years));

    swapTenors_ = std::vector<Period>();
    swapTenors_.push_back(Period(2, Years));
    swapTenors_.push_back(Period(10, Years));
    swapTenors_.push_back(Period(30, Years));

    strikeSpreads_ = std::vector<Rate>();
    strikeSpreads_.push_back(-0.020);
    strikeSpreads_.push_back(-0.005);
    strikeSpreads_.push_back(+0.000);
    strikeSpreads_.push_back(+0.005);
    strikeSpreads_.push_back(+0.020);

    Size nRows = optionTenors_.size()*swapTenors_.size();
    Size nCols = strikeSpreads_.size();
    volSpreadsMatrix_ = Matrix(nRows, nCols);
    volSpreadsMatrix_[0][0]=0.0599; volSpreadsMatrix_[0][1]=0.0049;
    volSpreadsMatrix_[0][2]=0.0000;
    volSpreadsMatrix_[0][3]=-0.0001; volSpreadsMatrix_[0][4]=0.0127;

    volSpreadsMatrix_[1][0]=0.0729; volSpreadsMatrix_[1][1]=0.0086;
    volSpreadsMatrix_[1][2]=0.0000;
    volSpreadsMatrix_[1][3]=-0.0024; volSpreadsMatrix_[1][4]=0.0098;

    volSpreadsMatrix_[2][0]=0.0738; volSpreadsMatrix_[2][1]=0.0102;
    volSpreadsMatrix_[2][2]=0.0000;
    volSpreadsMatrix_[2][3]=-0.0039; volSpreadsMatrix_[2][4]=0.0065;

    volSpreadsMatrix_[3][0]=0.0465; volSpreadsMatrix_[3][1]=0.0063;
    volSpreadsMatrix_[3][2]=0.0000;
    volSpreadsMatrix_[3][3]=-0.0032; volSpreadsMatrix_[3][4]=-0.0010;

    volSpreadsMatrix_[4][0]=0.0558; volSpreadsMatrix_[4][1]=0.0084;
    volSpreadsMatrix_[4][2]=0.0000;
    volSpreadsMatrix_[4][3]=-0.0050; volSpreadsMatrix_[4][4]=-0.0057;

    volSpreadsMatrix_[5][0]=0.0576; volSpreadsMatrix_[5][1]=0.0083;
    volSpreadsMatrix_[5][2]=0.0000;
    volSpreadsMatrix_[5][3]=-0.0043; volSpreadsMatrix_[5][4]=-0.0014;

    volSpreadsMatrix_[6][0]=0.0437; volSpreadsMatrix_[6][1]=0.0059;
    volSpreadsMatrix_[6][2]=0.0000;
    volSpreadsMatrix_[6][3]=-0.0030; volSpreadsMatrix_[6][4]=-0.0006;

    volSpreadsMatrix_[7][0]=0.0533; volSpreadsMatrix_[7][1]=0.0078;
    volSpreadsMatrix_[7][2]=0.0000;
    volSpreadsMatrix_[7][3]=-0.0045; volSpreadsMatrix_[7][4]=-0.0046;

    volSpreadsMatrix_[8][0]=0.0545; volSpreadsMatrix_[8][1]=0.0079;
    volSpreadsMatrix_[8][2]=0.0000;
    volSpreadsMatrix_[8][3]=-0.0042; volSpreadsMatrix_[8][4]=-0.0020;


    swapSettlementDays_ = 2;
    fixedLegFrequency_ = Annual;
    fixedLegConvention_ = Unadjusted;
    fixedLegDayCounter_ = Thirty360();
    termStructure_.linkTo(flatRate(referenceDate_, 0.05, Actual365Fixed()));
    iborIndex_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    shortTenor_ = 2;
    iborIndexShortTenor_ = boost::shared_ptr<IborIndex>(new
        Euribor3M(termStructure_));
    swapIndexBase_ = boost::shared_ptr<SwapIndex>(new
        SwapIndex("EurliborSwapFixA",
                  10*Years,
                  swapSettlementDays_,
                  iborIndex_->currency(),
                  calendar_,
                  Period(fixedLegFrequency_),
                  fixedLegConvention_,
                  iborIndex_->dayCounter(),
                  iborIndex_));

    vegaWeightedSmileFit_=false;

    // ATM Volatility structure
    std::vector<std::vector<Handle<Quote> > > atmVolsHandle_;
    atmVolsHandle_ = std::vector<std::vector<Handle<Quote> > >(nRowsAtmVols);
    Size i;
    for (i=0; i<nRowsAtmVols; i++){
        atmVolsHandle_[i] = std::vector<Handle<Quote> >(nColsAtmVols);
        for (Size j=0; j<nColsAtmVols; j++) {
            atmVolsHandle_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(atmVolMatrix_[i][j])));
        }
    }

    dayCounter_ = Actual365Fixed();

    atmVol_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(calendar_,
                                     atmOptionTenors_,
                                     atmSwapTenors_,
                                     atmVolsHandle_,
                                     dayCounter_,
                                     optionBDC_)));


    // Volatility Cube without smile
    std::vector<std::vector<Handle<Quote> > > parametersGuess(optionTenors_.size()*swapTenors_.size());
    for (i=0; i<optionTenors_.size()*swapTenors_.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);
    isParameterFixed[1]=true;

    std::vector<std::vector<Handle<Quote> > > nullVolSpreads(nRows);
    for (i=0; i<optionTenors_.size()*swapTenors_.size(); i++){
        nullVolSpreads[i] = std::vector<Handle<Quote> >(nCols);
        for (Size j=0; j<strikeSpreads_.size(); j++) {
            nullVolSpreads[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(0.)));
        }
    }

    boost::shared_ptr<SwaptionVolCube1>
    flatSwaptionVolatilityCube1(new SwaptionVolCube1(
        atmVol_,
        optionTenors_,
        swapTenors_,
        strikeSpreads_,
        nullVolSpreads,
        swapIndexBase_,
        vegaWeightedSmileFit_,
        parametersGuess,
        isParameterFixed,
        false));
    flatSwaptionVolatilityCube1_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(flatSwaptionVolatilityCube1));
    flatSwaptionVolatilityCube1_->enableExtrapolation();

    boost::shared_ptr<SwaptionVolCube2>
    flatSwaptionVolatilityCube2(new SwaptionVolCube2(atmVol_,
                                           optionTenors_,
                                           swapTenors_,
                                           strikeSpreads_,
                                           nullVolSpreads,
                                           swapIndexBase_,
                                           vegaWeightedSmileFit_));
    flatSwaptionVolatilityCube2_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(flatSwaptionVolatilityCube2));
    flatSwaptionVolatilityCube2_->enableExtrapolation();


    // Volatility Cube with smile
    volSpreads_ = std::vector<std::vector<Handle<Quote> > >(nRows);
    for (i=0; i<optionTenors_.size()*swapTenors_.size(); i++){
        volSpreads_[i] = std::vector<Handle<Quote> >(nCols);
        for (Size j=0; j<strikeSpreads_.size(); j++) {
            volSpreads_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(volSpreadsMatrix_[i][j])));
        }
    }

    boost::shared_ptr<SwaptionVolCube1>
    swaptionVolatilityCubeBySabr(new SwaptionVolCube1(
        atmVol_,
        optionTenors_,
        swapTenors_,
        strikeSpreads_,
        volSpreads_,
        swapIndexBase_,
        vegaWeightedSmileFit_,
        parametersGuess,
        isParameterFixed,
        false));
    swaptionVolatilityCubeBySabr_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(swaptionVolatilityCubeBySabr));
    swaptionVolatilityCubeBySabr_->enableExtrapolation();

    swaptionVolatilityStructures_ = std::vector<Handle<SwaptionVolatilityStructure> >();
    swaptionVolatilityStructures_.push_back(atmVol_);
    //swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCube1_);
    swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCube2_);
    swaptionVolatilityStructures_.push_back(swaptionVolatilityCubeBySabr_);

    modelOfYieldCurves_ = std::vector<GFunctionFactory::ModelOfYieldCurve>();
    modelOfYieldCurves_.push_back(GFunctionFactory::Standard);
    modelOfYieldCurves_.push_back(GFunctionFactory::ExactYield);
    modelOfYieldCurves_.push_back(GFunctionFactory::ParallelShifts);
    modelOfYieldCurves_.push_back(GFunctionFactory::NonParallelShifts);

    // Cms Schedules conventions
    fixedCmsConvention_ = Unadjusted;
    floatingCmsConvention_ = ModifiedFollowing;
    fixedCmsFrequency_ = Semiannual;
    floatingCmsFrequency_ = Semiannual;
    fixedCmsDayCount_ = Thirty360();

    // Cms valuation
    infiniteCap_ = 100.0;
    infiniteFloor_ = 0.0;
    gearing_ = 1.0;
    spread_ = 0.0;

    startDate_ = (referenceDate_+2*3600);
    paymentDate_ = (startDate_+365);
    endDate_ = (paymentDate_);
    settlementDays_ = 2;

    index_ = boost::shared_ptr<SwapIndex>(new
        SwapIndex("EurliborSwapFixA",
                10*Years,
                swapSettlementDays_,
                iborIndex_->currency(),
                calendar_,
                Period(fixedLegFrequency_),
                fixedLegConvention_,
                iborIndex_->dayCounter(),
                iborIndex_)
    );

    //test parameters
    rateTolerance_ = 2.0e-4;
    priceTolerance_ = 2.0e-4;

}


QL_END_TEST_LOCALS(CmsTest)

void CmsTest::testFairRate()  {

    BOOST_MESSAGE(
            "Testing fair-rate calculation for constant-maturity coupons...");

    SavedSettings backup;

    setup();

    Handle<Quote> nullMeanReversionQuote =
                    Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    for(Size h=0; h<modelOfYieldCurves_.size(); h++) {

        boost::shared_ptr<CmsCouponPricer> numericalPricer(new
            ConundrumPricerByNumericalIntegration(atmVol_,
                                                  modelOfYieldCurves_[h],
                                                  nullMeanReversionQuote,
                                                  0., 1.));
        boost::shared_ptr<CmsCouponPricer> analyticPricer(new
            ConundrumPricerByBlack(atmVol_,
                                   modelOfYieldCurves_[h],
                                   nullMeanReversionQuote));

        //Coupons
        CappedFlooredCmsCoupon coupon1(
            paymentDate_,1, startDate_, endDate_, settlementDays_,
            index_, gearing_, spread_,
            infiniteCap_, infiniteFloor_,
            startDate_, endDate_,
            iborIndex_->dayCounter());
        coupon1.setPricer(numericalPricer);

        CappedFlooredCmsCoupon coupon2(
            paymentDate_,1, startDate_, endDate_, settlementDays_,
            index_, gearing_, spread_,
            infiniteCap_, infiniteFloor_,
            startDate_, endDate_,
            iborIndex_->dayCounter());
        coupon2.setPricer(analyticPricer);

        //Computation
        const double rate1 = coupon1.rate();
        const double rate2 = coupon2.rate();
        const double difference =  rate2-rate1;

        if (std::fabs(difference) > rateTolerance_) {
            BOOST_ERROR("\n" <<
                        "startDate:\t" << startDate_ << "\n"
                        "rate1:\t" << io::rate(rate1) << "\n"
                        "rate2:\t" << io::rate(rate2) << "\n"
                        "difference:\t" << io::rate(difference) << "\n"
                        "tolerance: \t" << io::rate(rateTolerance_));
        }
    }
}

void CmsTest::testParity() {

    BOOST_MESSAGE("Testing put-call parity for constant-maturity coupons...");

    SavedSettings backup;

    setup();

    Handle<Quote> nullMeanReversionQuote =
                    Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));

    int priceIndex = 1;
    for (Size volStructureIndex = 0;
         volStructureIndex < swaptionVolatilityStructures_.size();
         volStructureIndex++) {

        for(Size modelOfYieldCurveIndex=0;
            modelOfYieldCurveIndex<modelOfYieldCurves_.size();
            modelOfYieldCurveIndex++) {

            std::vector<boost::shared_ptr<CmsCouponPricer> > pricers;
            {
                boost::shared_ptr<CmsCouponPricer> analyticPricer(
                    new ConundrumPricerByBlack(swaptionVolatilityStructures_[volStructureIndex],
                                               modelOfYieldCurves_[modelOfYieldCurveIndex],
                                               nullMeanReversionQuote));
                pricers.push_back(analyticPricer);

                boost::shared_ptr<CmsCouponPricer> numericalPricer(
                    new ConundrumPricerByNumericalIntegration(
                                                swaptionVolatilityStructures_[volStructureIndex],
                                                modelOfYieldCurves_[modelOfYieldCurveIndex],
                                                nullMeanReversionQuote,
                                                0, 1));
                pricers.push_back(numericalPricer);
            }

            for (Size pricerIndex=0; pricerIndex < pricers.size();
                 pricerIndex++) {

                CappedFlooredCmsCoupon swaplet(paymentDate_, 1,
                                  startDate_, endDate_, settlementDays_,
                                  index_, gearing_, spread_,
                                  infiniteCap_, infiniteFloor_,
                                  startDate_, endDate_,
                                  iborIndex_->dayCounter());
                swaplet.setPricer(pricers[pricerIndex]);
                Real strike = .02;

                for (Size strikeIndex = 0; strikeIndex < 10; strikeIndex++) {

                    strike += .005;
                    CappedFlooredCmsCoupon caplet(paymentDate_, 1,
                                     startDate_, endDate_, settlementDays_,
                                     index_, gearing_, spread_,
                                     strike, infiniteFloor_,
                                     startDate_, endDate_,
                                     iborIndex_->dayCounter());
                    caplet.setPricer(pricers[pricerIndex]);

                    CappedFlooredCmsCoupon floorlet(paymentDate_, 1,
                                       startDate_, endDate_, settlementDays_,
                                       index_, gearing_, spread_,
                                       infiniteCap_, strike,
                                       startDate_, endDate_,
                                       iborIndex_->dayCounter());
                    floorlet.setPricer(pricers[pricerIndex]);

                    //Computation
                    const double price1 = swaplet.price(termStructure_)
                        + swaplet.accrualPeriod()*strike*
                          termStructure_->discount(paymentDate_);
                    const double price2 = floorlet.price(termStructure_)
                        + caplet.price(termStructure_);

                    priceIndex++;

                    const double difference =  price1-price2;

                    if (std::fabs(difference) > priceTolerance_) {
                            BOOST_ERROR("\n" <<
                                "startDate:\t" << startDate_ << "\n"
                                "strike:\t" << strike << "\n"
                                "price1:\t" << io::rate(price1) << "\n"
                                "price2:\t" << io::rate(price2) << "\n"
                                "difference:\t" << io::rate(difference) << "\n"
                                "tolerance: \t" << io::rate(priceTolerance_));
                    }
                }
            }
        }
    }
}


void CmsTest::testCmsSwap() {

    BOOST_MESSAGE("Testing constant-maturity swaps...");

    SavedSettings backup;

    setup();

    swaptionVolatilityStructures_.clear();
    swaptionVolatilityStructures_.push_back(atmVol_);
    //swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCube1_);
    swaptionVolatilityStructures_.push_back(flatSwaptionVolatilityCube2_);

    std::vector<Size> swapLengths;
    swapLengths.push_back(1);
    swapLengths.push_back(5);
    swapLengths.push_back(6);
    swapLengths.push_back(10);

    int priceIndex = 1;
    Handle<Quote> nullMeanReversionQuote =
        Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));

    for (Size swapLengthIndex = 0; swapLengthIndex<swapLengths.size();
        swapLengthIndex++) {

        const int n = swapLengths[swapLengthIndex];

        std::vector<Real> meanReversions(n, 0);
        std::vector<Real> fixedNominals(n, 1);
        std::vector<Real> floatingNominals(n, 1);
        std::vector<Real> caps(n, infiniteCap_);
        std::vector<Real> floors(n, infiniteFloor_);
        std::vector<Real> fractions(n, gearing_);
        std::vector<Spread> baseRate(n, 0.0);

        for (Size volStructureIndex = 0;
             volStructureIndex < swaptionVolatilityStructures_.size();
             volStructureIndex++) {

            for (Size modelOfYieldCurveIndex=0;
                 modelOfYieldCurveIndex<modelOfYieldCurves_.size();
                 modelOfYieldCurveIndex++) {

                std::vector<boost::shared_ptr<CmsCouponPricer> > pricers;
                boost::shared_ptr<CmsCouponPricer> analyticPricer(
                    new ConundrumPricerByBlack(
                                    swaptionVolatilityStructures_[volStructureIndex],
                                    modelOfYieldCurves_[modelOfYieldCurveIndex],
                                    nullMeanReversionQuote));
                pricers.push_back(analyticPricer);

                boost::shared_ptr<CmsCouponPricer> numericalPricer(
                    new ConundrumPricerByNumericalIntegration(
                                swaptionVolatilityStructures_[volStructureIndex],
                                modelOfYieldCurves_[modelOfYieldCurveIndex],
                                nullMeanReversionQuote,
                                0, 1));
                pricers.push_back(numericalPricer);

                Date startDate = today_;
                startDate += Period(10, Days);
                Date maturityDate = startDate;
                maturityDate += Period(n, Years);

                Schedule fixedSchedule(startDate,maturityDate,
                                       Period(fixedCmsFrequency_),
                                       calendar_,fixedCmsConvention_,
                                       fixedLegConvention_,
                                       false,false);
                Schedule floatingSchedule(startDate,maturityDate,
                                          Period(floatingCmsFrequency_),
                                          calendar_,floatingCmsConvention_,
                                          fixedLegConvention_,
                                          false,false);

                std::vector<Real> prices;
                for (Size pricerIndex=0; pricerIndex<pricers.size();
                     pricerIndex++) {

                    Leg cmsLeg = CmsLeg(fixedNominals,
                                        fixedSchedule,
                                        index_,
                                        fixedCmsDayCount_,
                                        fixedCmsConvention_,
                                        std::vector<Natural>(1,settlementDays_),
                                        fractions,
                                        baseRate,
                                        caps,
                                        floors);
                    setCouponPricer(cmsLeg, pricers[pricerIndex]);

                    Leg floatingLeg = IborLeg(floatingNominals,
                                              floatingSchedule,
                                              iborIndex_,
                                              iborIndex_->dayCounter(),
                                              floatingCmsConvention_,
                                              std::vector<Natural>(1,settlementDays_));
                    boost::shared_ptr<IborCouponPricer>
                      fictitiousPricer(new BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
                    setCouponPricer(floatingLeg,fictitiousPricer);

                    boost::shared_ptr<Swap> swap(new
                        Swap(termStructure_, cmsLeg, floatingLeg));

                    Real price = swap->NPV();
                    prices.push_back(price);
                    priceIndex++;
                }

                const double difference =  prices[0]-prices[1];
                if (std::fabs(difference) > priceTolerance_) {
                    BOOST_ERROR("\n" <<
                                "difference:\t" << io::rate(difference) << "\n"
                                "tolerance: \t" << io::rate(priceTolerance_));
                }
            }
        }
    }
}

test_suite* CmsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cms tests");
    suite->add(BOOST_TEST_CASE(&CmsTest::testFairRate));
    suite->add(BOOST_TEST_CASE(&CmsTest::testParity));
    suite->add(BOOST_TEST_CASE(&CmsTest::testCmsSwap));
    return suite;
}

