/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/floatfloatswap.hpp>
#include <ql/instruments/floatfloatswaption.hpp>
#include <ql/instruments/nonstandardswaption.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dnonstandardswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dfloatfloatswaptionengine.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/models/shortrate/onefactormodels/markovfunctional.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/cashflows/lineartsrpricer.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

// helper function that prints a basket of calibrating swaptions to std::cout

void printBasket(
    const std::vector<ext::shared_ptr<BlackCalibrationHelper> > &basket) {
    std::cout << "\n" << std::left << std::setw(20) << "Expiry" << std::setw(20)
              << "Maturity" << std::setw(20) << "Nominal" << std::setw(14)
              << "Rate" << std::setw(12) << "Pay/Rec" << std::setw(14)
              << "Market ivol" << std::fixed << std::setprecision(6)
              << std::endl;
    std::cout << "===================="
                 "===================="
                 "===================="
                 "===================="
                 "==================" << std::endl;
    for (const auto& j : basket) {
        ext::shared_ptr<SwaptionHelper> helper = ext::dynamic_pointer_cast<SwaptionHelper>(j);
        Date endDate = helper->underlyingSwap()->fixedSchedule().dates().back();
        Real nominal = helper->underlyingSwap()->nominal();
        Real vol = helper->volatility()->value();
        Real rate = helper->underlyingSwap()->fixedRate();
        Date expiry = helper->swaption()->exercise()->date(0);
        Swap::Type type = helper->swaption()->type();
        std::ostringstream expiryString, endDateString;
        expiryString << expiry;
        endDateString << endDate;
        std::cout << std::setw(20) << expiryString.str() << std::setw(20)
                  << endDateString.str() << std::setw(20) << nominal
                  << std::setw(14) << rate << std::setw(12)
                  << (type == Swap::Payer ? "Payer" : "Receiver")
                  << std::setw(14) << vol << std::endl;
    }
}

// helper function that prints the result of a model calibration to std::cout

void printModelCalibration(
    const std::vector<ext::shared_ptr<BlackCalibrationHelper> > &basket,
    const Array &volatility) {

    std::cout << "\n" << std::left << std::setw(20) << "Expiry" << std::setw(14)
              << "Model sigma" << std::setw(20) << "Model price"
              << std::setw(20) << "market price" << std::setw(14)
              << "Model ivol" << std::setw(14) << "Market ivol" << std::fixed
              << std::setprecision(6) << std::endl;
    std::cout << "===================="
                 "===================="
                 "===================="
                 "===================="
                 "====================" << std::endl;

    for (Size j = 0; j < basket.size(); ++j) {
        ext::shared_ptr<SwaptionHelper> helper =
            ext::dynamic_pointer_cast<SwaptionHelper>(basket[j]);
        Date expiry = helper->swaption()->exercise()->date(0);
        std::ostringstream expiryString;
        expiryString << expiry;
        std::cout << std::setw(20) << expiryString.str() << std::setw(14)
                  << volatility[j] << std::setw(20) << basket[j]->modelValue()
                  << std::setw(20) << basket[j]->marketValue() << std::setw(14)
                  << basket[j]->impliedVolatility(basket[j]->modelValue(), 1E-6,
                                                  1000, 0.0, 2.0)
                  << std::setw(14) << basket[j]->volatility()->value()
                  << std::endl;
    }
    if (volatility.size() > basket.size()) // only for markov model
        std::cout << std::setw(20) << " " << volatility.back() << std::endl;
}


// here the main part of the code starts

int main(int argc, char *argv[]) {

    try {

        std::cout << "\nGaussian1dModel Examples" << std::endl;

        std::cout << "\nThis is some example code showing how to use the GSR "
                     "\n(Gaussian short rate) and Markov Functional model."
                  << std::endl;

        Date refDate(30, April, 2014);
        Settings::instance().evaluationDate() = refDate;

        std::cout << "\nThe evaluation date for this example is set to "
                  << Settings::instance().evaluationDate() << std::endl;

        Real forward6mLevel = 0.025;
        Real oisLevel = 0.02;

        Handle<Quote> forward6mQuote(
            ext::make_shared<SimpleQuote>(forward6mLevel));
        Handle<Quote> oisQuote(ext::make_shared<SimpleQuote>(oisLevel));

        Handle<YieldTermStructure> yts6m(ext::make_shared<FlatForward>(
            0, TARGET(), forward6mQuote, Actual365Fixed()));
        Handle<YieldTermStructure> ytsOis(ext::make_shared<FlatForward>(
            0, TARGET(), oisQuote, Actual365Fixed()));

        ext::shared_ptr<IborIndex> euribor6m =
            ext::make_shared<Euribor>(6 * Months, yts6m);

        std::cout
            << "\nWe assume a multicurve setup, for simplicity with flat yield "
               "\nterm structures. The discounting curve is an Eonia curve at"
               "\na level of " << oisLevel
            << " and the forwarding curve is an Euribior 6m curve"
            << "\nat a level of " << forward6mLevel << std::endl;

        Real volLevel = 0.20;
        Handle<Quote> volQuote(ext::make_shared<SimpleQuote>(volLevel));
        Handle<SwaptionVolatilityStructure> swaptionVol(
            ext::make_shared<ConstantSwaptionVolatility>(
                0, TARGET(), ModifiedFollowing, volQuote, Actual365Fixed()));

        std::cout
            << "\nFor the volatility we assume a flat swaption volatility at "
            << volLevel << std::endl;

        Real strike = 0.04;
        std::cout << "\nWe consider a standard 10y bermudan payer swaption "
                     "\nwith yearly exercises at a strike of " << strike
                  << std::endl;

        Date effectiveDate = TARGET().advance(refDate, 2 * Days);
        Date maturityDate = TARGET().advance(effectiveDate, 10 * Years);

        Schedule fixedSchedule(effectiveDate, maturityDate, 1 * Years, TARGET(),
                               ModifiedFollowing, ModifiedFollowing,
                               DateGeneration::Forward, false);
        Schedule floatingSchedule(effectiveDate, maturityDate, 6 * Months,
                                  TARGET(), ModifiedFollowing,
                                  ModifiedFollowing, DateGeneration::Forward,
                                  false);

        ext::shared_ptr<NonstandardSwap> underlying =
            ext::make_shared<NonstandardSwap>(VanillaSwap(
                Swap::Payer, 1.0, fixedSchedule, strike, Thirty360(Thirty360::BondBasis),
                floatingSchedule, euribor6m, 0.00, Actual360()));

        std::vector<Date> exerciseDates;
        for (Size i = 1; i < 10; ++i)
            exerciseDates.push_back(
                TARGET().advance(fixedSchedule[i], -2 * Days));

        ext::shared_ptr<Exercise> exercise =
            ext::make_shared<BermudanExercise>(exerciseDates, false);
        ext::shared_ptr<NonstandardSwaption> swaption =
            ext::make_shared<NonstandardSwaption>(underlying, exercise);

        std::cout
            << "\nThe model is a one factor Hull White model with piecewise "
               "\nvolatility adapted to our exercise dates." << std::endl;

        std::vector<Date> stepDates(exerciseDates.begin(),
                                    exerciseDates.end() - 1);
        std::vector<Real> sigmas(stepDates.size() + 1, 0.01);
        Real reversion = 0.01;

        std::cout << "\nThe reversion is just kept constant at a level of "
                  << reversion << std::endl;

        std::cout
            << "\nThe model's curve is set to the 6m forward curve. Note that "
               "\nthe model adapts automatically to other curves where "
               "appropriate "
               "\n(e.g. if an index requires a different forwarding curve) or "
               "\nwhere explicitly specified (e.g. in a swaption pricing "
               "engine)." << std::endl;

        ext::shared_ptr<Gsr> gsr = ext::make_shared<Gsr>(
            yts6m, stepDates, sigmas, reversion);

        ext::shared_ptr<PricingEngine> swaptionEngine =
            ext::make_shared<Gaussian1dSwaptionEngine>(gsr, 64, 7.0, true,
                                                         false, ytsOis);
        ext::shared_ptr<PricingEngine> nonstandardSwaptionEngine =
            ext::make_shared<Gaussian1dNonstandardSwaptionEngine>(
                gsr, 64, 7.0, true, false, Handle<Quote>(), ytsOis);

        swaption->setPricingEngine(nonstandardSwaptionEngine);

        std::cout
            << "\nThe engine can generate a calibration basket in two modes."
               "\nThe first one is called Naive and generates ATM swaptions "
               "adapted to"
               "\nthe exercise dates of the swaption and its maturity date"
            << std::endl;

        std::cout << "\nThe resulting basket looks as follows:" << std::endl;

        ext::shared_ptr<SwapIndex> swapBase =
            ext::make_shared<EuriborSwapIsdaFixA>(10 * Years, yts6m, ytsOis);

        
        std::vector<ext::shared_ptr<BlackCalibrationHelper> > basket =
            swaption->calibrationBasket(swapBase, *swaptionVol,
                                        BasketGeneratingEngine::Naive);
        printBasket(basket);
        

        std::cout
            << "\nLet's calibrate our model to this basket. We use a "
               "specialized"
               "\ncalibration method calibrating the sigma function one by one "
               "to"
               "\nthe calibrating vanilla swaptions. The result of this is as "
               "follows:" << std::endl;

        for (auto& i : basket)
            i->setPricingEngine(swaptionEngine);

        LevenbergMarquardt method;
        EndCriteria ec(1000, 10, 1E-8, 1E-8,
                       1E-8); // only max iterations use actually used by LM

        
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        

        printModelCalibration(basket, gsr->volatility());
        

        std::cout << "\nFinally we price our bermudan swaption in the "
                     "calibrated model:" << std::endl;

        
        Real npv = swaption->NPV();
        

        std::cout << "\nBermudan swaption NPV (ATM calibrated GSR) = "
                  << std::fixed << std::setprecision(6) << npv << std::endl;
        

        std::cout << "\nThere is another mode to generate a calibration basket called"
                     "\nMaturityStrikeByDeltaGamma. This means that the maturity,"
                     "\nthe strike and the nominal of the calibrating swaptions are"
                     "\nobtained matching the NPV, first derivative and second derivative"
                     "\nof the swap you will exercise into at at each bermudan call date."
                     "\nThe derivatives are taken with respect to the model's state variable."
                     "\nLet's try this in our case."
                  << std::endl;

        
        basket = swaption->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        

        printBasket(basket);
        

        std::cout
            << "\nThe calibrated nominal is close to the exotics nominal."
               "\nThe expiries and maturity dates of the vanillas are the same"
               "\nas in the case above. The difference is the strike which"
               "\nis now equal to the exotics strike." << std::endl;

        std::cout << "\nLet's see how this affects the exotics npv. The "
                     "\nrecalibrated model is:" << std::endl;

        for (auto& i : basket)
            i->setPricingEngine(swaptionEngine);


        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        

        printModelCalibration(basket, gsr->volatility());
        

        std::cout << "\nAnd the bermudan's price becomes:" << std::endl;

        
        npv = swaption->NPV();
        

        std::cout << "\nBermudan swaption NPV (deal strike calibrated GSR) = "
                  << std::setprecision(6) << npv << std::endl;

        

        std::cout
            << "\nWe can do more complicated things, let's e.g. modify the"
               "\nnominal schedule to be linear amortizing and see what"
               "\nthe effect on the generated calibration basket is:"
            << std::endl;

        std::vector<Real> nominalFixed, nominalFloating;
        for (Size i = 0; i < fixedSchedule.size() - 1; ++i) {
            Real tmpNom = 1.0 - (Real)i / (fixedSchedule.size() - 1);
            nominalFixed.push_back(tmpNom);
            nominalFloating.push_back(tmpNom);
            nominalFloating.push_back(
                tmpNom); // we use that the swap is 6m vs. 1y here
        }
        std::vector<Real> strikes(nominalFixed.size(), strike);

        ext::shared_ptr<NonstandardSwap> underlying2(new NonstandardSwap(
            Swap::Payer, nominalFixed, nominalFloating, fixedSchedule,
            strikes, Thirty360(Thirty360::BondBasis), floatingSchedule, euribor6m, 1.0, 0.0,
            Actual360()));
        ext::shared_ptr<NonstandardSwaption> swaption2 =
            ext::make_shared<NonstandardSwaption>(underlying2, exercise);

        swaption2->setPricingEngine(nonstandardSwaptionEngine);

        
        basket = swaption2->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        

        printBasket(basket);
        

        std::cout << "\nThe notional is weighted over the underlying exercised "
                     "\ninto and the maturity is adjusted downwards. The rate"
                     "\non the other hand is not affected." << std::endl;

        std::cout
            << "\nYou can also price exotic bond's features. If you have e.g. a"
               "\nbermudan callable fixed bond you can set up the call right "
               "\nas a swaption to enter into a one leg swap with notional"
               "\nreimbursement at maturity."
               "\nThe exercise should then be written as a rebated exercise"
               "\npaying the notional in case of exercise." << std::endl;

        std::cout << "\nThe calibration basket looks like this:" << std::endl;

        std::vector<Real> nominalFixed2(nominalFixed.size(), 1.0);
        std::vector<Real> nominalFloating2(nominalFloating.size(),
                                           0.0); // null the second leg

        ext::shared_ptr<NonstandardSwap> underlying3(new NonstandardSwap(
            Swap::Receiver, nominalFixed2, nominalFloating2,
            fixedSchedule, strikes, Thirty360(Thirty360::BondBasis), floatingSchedule, euribor6m,
            1.0, 0.0, Actual360(), false,
            true)); // final capital exchange

        ext::shared_ptr<RebatedExercise> exercise2 =
            ext::make_shared<RebatedExercise>(*exercise, -1.0, 2, TARGET());

        ext::shared_ptr<NonstandardSwaption> swaption3 =
            ext::make_shared<NonstandardSwaption>(underlying3, exercise2);

        ext::shared_ptr<SimpleQuote> oas0 =
            ext::make_shared<SimpleQuote>(0.0);
        ext::shared_ptr<SimpleQuote> oas100 =
            ext::make_shared<SimpleQuote>(0.01);
        RelinkableHandle<Quote> oas(oas0);

        ext::shared_ptr<PricingEngine> nonstandardSwaptionEngine2 =
            ext::make_shared<Gaussian1dNonstandardSwaptionEngine>(
                gsr, 64, 7.0, true, false, oas); // change discounting to 6m

        swaption3->setPricingEngine(nonstandardSwaptionEngine2);

        

        basket = swaption3->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        

        printBasket(basket);
        

        std::cout
            << "\nNote that nominals are not exactly 1.0 here. This is"
            << "\nbecause we do our bond discounting on 6m level while"
            << "\nthe swaptions are still discounted on OIS level."
            << "\n(You can try this by changing the OIS level to the "
            << "\n6m level, which will produce nominals near 1.0)."
            << "\nThe npv of the call right is (after recalibrating the model)"
            << std::endl;

        for (auto& i : basket)
            i->setPricingEngine(swaptionEngine);


        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        Real npv3 = swaption3->NPV();
        

        std::cout << "\nBond's bermudan call right npv = "
                  << std::setprecision(6) << npv3 << std::endl;
        

        std::cout
            << "\nUp to now, no credit spread is included in the pricing."
               "\nWe can do so by specifying an oas in the pricing engine."
               "\nLet's set the spread level to 100bp and regenerate"
               "\nthe calibration basket." << std::endl;

        oas.linkTo(oas100);

        
        basket = swaption3->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        
        printBasket(basket);
        

        std::cout
            << "\nThe adjusted basket takes the credit spread into account."
               "\nThis is consistent to a hedge where you would have a"
               "\nmargin on the float leg around 100bp,too." << std::endl;

        std::cout << "\nThe npv becomes:" << std::endl;

        for (auto& i : basket)
            i->setPricingEngine(swaptionEngine);


        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        Real npv4 = swaption3->NPV();
        

        std::cout << "\nBond's bermudan call right npv (oas = 100bp) = "
                  << std::setprecision(6) << npv4 << std::endl;
        

        std::cout
            << "\nThe next instrument we look at is a CMS 10Y vs Euribor "
               "\n6M swaption. The maturity is again 10 years and the option"
               "\nis exercisable on a yearly basis" << std::endl;

        ext::shared_ptr<FloatFloatSwap> underlying4(new FloatFloatSwap(
                Swap::Payer, 1.0, 1.0, fixedSchedule, swapBase,
                Thirty360(Thirty360::BondBasis), floatingSchedule, euribor6m, Actual360(), false,
                false, 1.0, 0.0, Null<Real>(), Null<Real>(), 1.0, 0.0010));

        ext::shared_ptr<FloatFloatSwaption> swaption4 =
            ext::make_shared<FloatFloatSwaption>(underlying4, exercise);

        ext::shared_ptr<Gaussian1dFloatFloatSwaptionEngine>
            floatSwaptionEngine(new Gaussian1dFloatFloatSwaptionEngine(
                    gsr, 64, 7.0, true, false, Handle<Quote>(), ytsOis, true));

        swaption4->setPricingEngine(floatSwaptionEngine);

        std::cout
            << "\nSince the underlying is quite exotic already, we start with"
               "\npricing this using the LinearTsrPricer for CMS coupon "
               "estimation" << std::endl;

        Handle<Quote> reversionQuote(
            ext::make_shared<SimpleQuote>(reversion));

        const Leg &leg0 = underlying4->leg(0);
        const Leg &leg1 = underlying4->leg(1);
        ext::shared_ptr<CmsCouponPricer> cmsPricer =
            ext::make_shared<LinearTsrPricer>(swaptionVol, reversionQuote);
        ext::shared_ptr<IborCouponPricer> iborPricer(new BlackIborCouponPricer);

        setCouponPricer(leg0, cmsPricer);
        setCouponPricer(leg1, iborPricer);

        ext::shared_ptr<PricingEngine> swapPricer =
            ext::make_shared<DiscountingSwapEngine>(ytsOis);

        underlying4->setPricingEngine(swapPricer);

        
        Real npv5 = underlying4->NPV();
        

        std::cout << "Underlying CMS Swap NPV = " << std::setprecision(6)
                  << npv5 << std::endl;
        std::cout << "       CMS     Leg  NPV = " << underlying4->legNPV(0)
                  << std::endl;
        std::cout << "       Euribor Leg  NPV = " << underlying4->legNPV(1)
                  << std::endl;

        

        std::cout << "\nWe generate a naive calibration basket and calibrate "
                     "\nthe GSR model to it:" << std::endl;

        
        basket = swaption4->calibrationBasket(swapBase, *swaptionVol,
                                              BasketGeneratingEngine::Naive);
        for (auto& i : basket)
            i->setPricingEngine(swaptionEngine);
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        

        printBasket(basket);
        printModelCalibration(basket, gsr->volatility());
        

        std::cout << "\nThe npv of the bermudan swaption is" << std::endl;

        
        Real npv6 = swaption4->NPV();
        

        std::cout << "\nFloat swaption NPV (GSR) = " << std::setprecision(6)
                  << npv6 << std::endl;
        

        std::cout << "\nIn this case it is also interesting to look at the "
                     "\nunderlying swap npv in the GSR model." << std::endl;

        std::cout << "\nFloat swap NPV (GSR) = " << std::setprecision(6)
                  << swaption4->result<Real>("underlyingValue") << std::endl;

        std::cout << "\nNot surprisingly, the underlying is priced differently"
                     "\ncompared to the LinearTsrPricer, since a different"
                     "\nsmile is implied by the GSR model." << std::endl;

        std::cout << "\nThis is exactly where the Markov functional model"
                  << "\ncomes into play, because it can calibrate to any"
                  << "\ngiven underlying smile (as long as it is arbitrage"
                  << "\nfree). We try this now. Of course the usual use case"
                  << "\nis not to calibrate to a flat smile as in our simple"
                  << "\nexample, still it should be possible, of course..."
                  << std::endl;

        std::vector<Date> markovStepDates(exerciseDates.begin(),
                                          exerciseDates.end());
        const std::vector<Date>& cmsFixingDates(markovStepDates);
        std::vector<Real> markovSigmas(markovStepDates.size() + 1, 0.01);
        std::vector<Period> tenors(cmsFixingDates.size(), 10 * Years);
        ext::shared_ptr<MarkovFunctional> markov =
            ext::make_shared<MarkovFunctional>(
                yts6m, reversion, markovStepDates, markovSigmas, swaptionVol,
                cmsFixingDates, tenors, swapBase,
                MarkovFunctional::ModelSettings().withYGridPoints(16));

        ext::shared_ptr<Gaussian1dSwaptionEngine> swaptionEngineMarkov =
            ext::make_shared<Gaussian1dSwaptionEngine>(markov, 8, 5.0, true,
                                                         false, ytsOis);
        ext::shared_ptr<Gaussian1dFloatFloatSwaptionEngine>
            floatEngineMarkov =
                ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
                    markov, 16, 7.0, true, false, Handle<Quote>(), ytsOis,
                    true);

        swaption4->setPricingEngine(floatEngineMarkov);

        
        Real npv7 = swaption4->NPV();
        

        std::cout << "\nThe option npv is the markov model is:" << std::endl;

        std::cout << "\nFloat swaption NPV (Markov) = " << std::setprecision(6)
                  << npv7 << std::endl;
        

        std::cout << "\nThis is not too far from the GSR price." << std::endl;

        std::cout << "\nMore interesting is the question how well the Markov"
                  << "\nmodel did its job to match our input smile. For this"
                  << "\nwe look at the underlying npv under the Markov model"
                  << std::endl;

        std::cout << "\nFloat swap NPV (Markov) = " << std::setprecision(6)
                  << swaption4->result<Real>("underlyingValue") << std::endl;

        std::cout << "\nThis is closer to our terminal swap rate model price."
                     "\nA perfect match is not expected anyway, because the"
                     "\ndynamics of the underlying rate in the linear"
                     "\nmodel is different from the Markov model, of"
                     "\ncourse." << std::endl;

        std::cout << "\nThe Markov model can not only calibrate to the"
                     "\nunderlying smile, but has at the same time a"
                     "\nsigma function (similar to the GSR model) which"
                     "\ncan be used to calibrate to a second instrument"
                     "\nset. We do this here to calibrate to our coterminal"
                     "\nATM swaptions from above." << std::endl;

        std::cout << "\nThis is a computationally demanding task, so"
                     "\ndepending on your machine, this may take a"
                     "\nwhile now..." << std::endl;

        for (auto& i : basket)
            i->setPricingEngine(swaptionEngineMarkov);


        markov->calibrate(basket, method, ec);
        

        printModelCalibration(basket, markov->volatility());
        

        std::cout << "\nNow let's have a look again at the underlying pricing."
                     "\nIt shouldn't have changed much, because the underlying"
                     "\nsmile is still matched." << std::endl;

        
        Real npv8 = swaption4->result<Real>("underlyingValue");
        
        std::cout << "\nFloat swap NPV (Markov) = " << std::setprecision(6)
                  << npv8 << std::endl;
        

        std::cout << "\nThis is close to the previous value as expected."
                  << std::endl;

        std::cout << "\nAs a final remark we note that the calibration to"
                  << "\ncoterminal swaptions is not particularly reasonable"
                  << "\nhere, because the european call rights are not"
                  << "\nwell represented by these swaptions."
                  << "\nSecondly, our CMS swaption is sensitive to the"
                  << "\ncorrelation between the 10y swap rate and the"
                  << "\nEuribor 6M rate. Since the Markov model is one factor"
                  << "\nit will most probably underestimate the market value"
                  << "\nby construction." << std::endl;

        std::cout << "\nThat was it. Thank you for running this demo. Bye."
                  << std::endl;

    } catch (const QuantLib::Error& e) {
        std::cout << "terminated with a ql exception: " << e.what()
                  << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "terminated with a general exception: " << e.what()
                  << std::endl;
        return 1;
    }
}
