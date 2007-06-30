/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "utilities.hpp"
#include "hybridhestonhullwhiteprocess.hpp"

#include <ql/time/schedule.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/termstructures/yieldcurves/zerocurve.hpp>

#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>

#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mchestonhullwhiteengine.hpp>
#include <ql/pricingengines/vanilla/analyticbsmhullwhiteengine.hpp>

#include <boost/bind.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


QL_BEGIN_TEST_LOCALS(HybridHestonHullWhiteProcessTest)

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(HybridHestonHullWhiteProcessTest)



void HybridHestonHullWhiteProcessTest::testBsmHullWhiteEngine() {
    BOOST_MESSAGE("Testing European option pricing for a BSM process"
                  " with one factor Hull-White Model...");

    SavedSettings backup;

    DayCounter dc = Actual365Fixed();

    const Date today = Date::todaysDate();
    const Date maturity = today + Period(20, Years);

    const Handle<Quote> spot(
                         boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0525));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<HullWhite> hullWhiteModel(
        new HullWhite(Handle<YieldTermStructure>(rTS), 0.00883, 0.00526));

    boost::shared_ptr<StochasticProcess> stochProcess(
                      new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

    Real fwd = spot->value()*qTS->discount(maturity)/rTS->discount(maturity);
    boost::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Call, fwd));

    EuropeanOption option(stochProcess, payoff, exercise);
        
    const Real tol = 1e-8;
    const Real corr[] = {-0.75, -0.25, 0.0, 0.25, 0.75 };
    const Volatility expectedVol[] = { 0.217064577, 0.243995801, 
                                       0.256402830, 0.268236596, 0.290461343 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        boost::shared_ptr<PricingEngine> bsmhwEngine(
                    new AnalyticBSMHullWhiteEngine(corr[i], hullWhiteModel));  
 
        option.setPricingEngine(bsmhwEngine);
        const Real npv = option.NPV();

        const Handle<BlackVolTermStructure> compVolTS( 
                                        flatVol(today, expectedVol[i], dc));

        EuropeanOption comp(
            boost::shared_ptr<StochasticProcess>(
                new BlackScholesMertonProcess(spot, qTS, rTS, compVolTS)),
            payoff, exercise);

        const Volatility impliedVol=comp.impliedVolatility(npv, 1e-10, 100);

        if (std::fabs(impliedVol - expectedVol[i]) > tol) {
            BOOST_FAIL("Failed to reproduce implied volatility"
                       << "\n    calculated: " << impliedVol
                       << "\n    expected  : " << expectedVol[i]);
        }
        if (std::fabs((comp.NPV() - npv)/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs(comp.delta() - option.delta()) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.gamma() - option.gamma())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.theta() - option.theta())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
        if (std::fabs((comp.vega() - option.vega())/npv) > tol) {
            BOOST_FAIL("Failed to reproduce NPV"
                       << "\n    calculated: " << npv
                       << "\n    expected  : " << comp.NPV());
        }
   }
}

void HybridHestonHullWhiteProcessTest::testZeroBondPricing() {
    BOOST_MESSAGE("Testing Monte-Carlo Zero Bond Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates;

    dates.push_back(today);
    rates.push_back(0.02);
    times.push_back(0.0);
    for (Size i=120; i < 240; ++i) {
        dates.push_back(today+Period(i, Months));
        rates.push_back(0.02 + 0.02*std::exp(std::sin(i/8.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }
    
    const Date maturity = dates.back() + Period(10, Years);
    dates.push_back(maturity);
    rates.push_back(0.04);
    times.push_back(dc.yearFraction(today, dates.back()));

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));

    const Handle<YieldTermStructure> ts(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> ds(flatRate(today, 0.0, dc));

    const boost::shared_ptr<HestonProcess> hestonProcess(
                   new HestonProcess(ts, ds, s0, 0.02, 1.0, 0.02, 0.5, -0.8));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
                   new HullWhiteForwardProcess(ts, 0.05, 0.05));
    hwProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));
    const boost::shared_ptr<HullWhite> hwModel(new HullWhite(ts, 0.05, 0.05));

    const boost::shared_ptr<JointStochasticProcess> jointProcess(
        new HybridHestonHullWhiteProcess(hestonProcess, hwProcess, -0.4, 5));

    TimeGrid grid(times.begin(), times.end()-1);

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
    
    BigNatural seed = 1234;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                              jointProcess->factors()*(grid.size()-1), seed);
    
    MultiPathGenerator<rsg_type> generator(jointProcess, grid, rsg, false);
    std::vector<GeneralStatistics> zeroStat(90);
    std::vector<GeneralStatistics> optionStat(90);

    const Size nrTrails = 1000;
    const Size optionTenor = 24;
    const DiscountFactor strike = 0.5;

    std::vector<DiscountFactor> tmpZero(90);
    std::vector<DiscountFactor> tmpOption(90);
    
    for (Size i=0; i < nrTrails; ++i) {
        const bool antithetic = (i%2);
        sample_type path = (!antithetic) ? generator.next() 
                                         : generator.antithetic();

        for (Size j=1; j < 90; ++j) {
            const Time t = grid[j];            // zero end and option maturity
            const Time T = grid[j+optionTenor];// maturity of zero bond 
                                               // of option

            Array states(5);
            Array optionStates(5);
            for (Size k=0; k < jointProcess->size(); ++k) {
                states[k]       = path.value[k][j];
                optionStates[k] = path.value[k][j+optionTenor];
            }

            const DiscountFactor zeroBond 
                = 1.0/jointProcess->numeraire(t, states);
            const DiscountFactor zeroOption = zeroBond
                * std::max(0.0, hwModel->discountBond(t, T, states[4])-strike);

            if (antithetic) {
                zeroStat[j].add(0.5*(tmpZero[j] + zeroBond));
                optionStat[j].add(0.5*(tmpOption[j] + zeroOption));
            }
            else {
                tmpZero[j] = zeroBond;
                tmpOption[j] = zeroOption;
            }
        }
    }

    for (Size j=1; j < 90; ++j) {
        const Time t = grid[j];
        Real calculated = zeroStat[j].mean();
        Real error = zeroStat[j].errorEstimate();
        Real expected = ts->discount(t);

        if (std::fabs(calculated - expected) > 3*error) {
            BOOST_ERROR("Failed to reproduce expected zero bond prices"
                        << "\n   t:          " << t
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
        }

        const Time T = grid[j+optionTenor];

        calculated = optionStat[j].mean();
        error = optionStat[j].errorEstimate();
        expected = hwModel->discountBondOption(Option::Call, strike, t, T);

        if (std::fabs(calculated - expected) > 3*error) {
            BOOST_ERROR("Failed to reproduce expected zero bond option prices"
                        << "\n   t:          " << t
                        << "\n   T:          " << T
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
        }
    }
}

void HybridHestonHullWhiteProcessTest::testMcVanillaPricing() {
    BOOST_MESSAGE("Testing Monte-Carlo Vanilla Option Pricing...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    const Date today = Date::todaysDate();

    // construct a strange yield curve to check drifts and discounting
    // of the joint stochastic process

    std::vector<Date> dates;
    std::vector<Time> times;
    std::vector<Rate> rates, divRates;

    for (Size i=0; i <= 40; ++i) {
        dates.push_back(today+Period(i, Years));
        rates.push_back(0.01 + 0.02*std::exp(std::sin(i/4.0)));
        divRates.push_back(0.02 + 0.01*std::exp(std::sin(i/5.0)));
        times.push_back(dc.yearFraction(today, dates.back()));
    }

    const Date maturity = today + Period(20, Years);

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));
    const Handle<YieldTermStructure> rTS(
       boost::shared_ptr<YieldTermStructure>(new ZeroCurve(dates, rates, dc)));
    const Handle<YieldTermStructure> qTS(       
       boost::shared_ptr<YieldTermStructure>(
                                          new ZeroCurve(dates, divRates, dc)));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<StochasticProcess> bsmProcess(
              new BlackScholesMertonProcess(s0, qTS, rTS, volTS));
    const boost::shared_ptr<HestonProcess> hestonProcess(
             new HestonProcess(rTS, qTS, s0, 0.0625, 0.5, 0.0625, 1e-5, 0));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
              new HullWhiteForwardProcess(rTS, 0.01, 0.01));
    hwProcess->setForwardMeasureTime(dc.yearFraction(today, maturity));
    
    const Real tol = 0.1;
    const Real corr[] = {-0.9, -0.5, 0.0, 0.5, 0.9 };
    const Real strike[] = { 100 };

    for (Size i=0; i < LENGTH(corr); ++i) {
        for (Size j=0; j < LENGTH(strike); ++j) {
            const boost::shared_ptr<JointStochasticProcess> jointProcess(
                new HybridHestonHullWhiteProcess(hestonProcess, 
                                                 hwProcess, corr[i], 5));

            const boost::shared_ptr<StrikedTypePayoff> payoff(
                               new PlainVanillaPayoff(Option::Put, strike[j]));
            const boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(maturity));

            VanillaOption optionHestonHW(jointProcess, payoff, exercise,
                boost::shared_ptr<PricingEngine>(
                    new MCHestonHullWhiteEngine<PseudoRandom>(
                                          5, Null<Size>(), true, true, 1, 
                                          tol, Null<Size>(), 42)));

            const boost::shared_ptr<HullWhite> hwModel(         
                        new HullWhite(Handle<YieldTermStructure>(rTS),
                                      hwProcess->a(), hwProcess->sigma()));
 
            VanillaOption optionBsmHW(bsmProcess, payoff, exercise,
                boost::shared_ptr<PricingEngine>(
                          new AnalyticBSMHullWhiteEngine(corr[i], hwModel)));

            const Real calculated = optionHestonHW.NPV();
            const Real error      = optionHestonHW.errorEstimate();
            const Real expected   = optionBsmHW.NPV();

            if (std::fabs(calculated - expected) > 3*error) {
                BOOST_ERROR("Failed to reproduce BSM-HW vanilla prices"
                        << "\n   corr:       " << corr[i]
                        << "\n   strike:     " << strike[j]
                        << "\n   calculated: " << calculated
                        << "\n   error:      " << error
                        << "\n   expected:   " << expected);
            }
        }
    }
}

void HybridHestonHullWhiteProcessTest::testCallableEquityPricing() {
    BOOST_MESSAGE("Testing the pricing of a callable equity product...");

    SavedSettings backup;

    /* 
       for the definition of the example product see
       Alexander Giese, On the Pricing of Auto-Callable Equity
       Structures in the Presence of Stochastic Volatility and
       Stochastic Interest Rates .
       http://workshop.mathfinance.de/2006/papers/giese/slides.pdf
    */
        
    const Size maturity = 7;
    DayCounter dc = Actual365Fixed();
    const Date today = Date::todaysDate();

    Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    const boost::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, spot, 0.0625, 1.0, 0.0625, 1e-4, 0.0));
    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
            new HullWhiteForwardProcess(rTS, 0.00883, 0.00526));
    hwProcess->setForwardMeasureTime(
                      dc.yearFraction(today, today+Period(maturity+1, Years)));

    const boost::shared_ptr<JointStochasticProcess> jointProcess(
        new HybridHestonHullWhiteProcess(hestonProcess, hwProcess, -0.4, 5));

    Schedule schedule(today, today + Period(maturity, Years),
                      Period(1, Years), TARGET(),
                      Following, Following, false, false);
                      
    std::vector<Time> times(maturity+1);
    std::transform(schedule.begin(), schedule.end(), times.begin(),
                   boost::bind(&Actual365Fixed::yearFraction,
                               dc, today, _1, Date(), Date()));

    for (Size i=0; i<=maturity; ++i) times[i] = i;

    TimeGrid grid(times.begin(), times.end());

    std::vector<Real> redemption(maturity);
    for (Size i=0; i < maturity; ++i) {
        redemption[i] = 1.07 + 0.03*i;
    }

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
    
    BigNatural seed = 42;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                              jointProcess->factors()*(grid.size()-1), seed);
    
    MultiPathGenerator<rsg_type> generator(jointProcess, grid, rsg, false);
    GeneralStatistics stat;

    Real antitheticPayoff=0;
    const Size nrTrails = 5000;
    for (Size i=0; i < nrTrails; ++i) {
        const bool antithetic = (i%2);

        sample_type path = (!antithetic) ? generator.next() 
                                         : generator.antithetic();

        Real payoff=0;
        for (Size j=1; j <= maturity; ++j) {
            if (path.value[0][j] > spot->value()) {
                Array states(5);
                for (Size k=0; k < 5; ++k) {
                    states[k] = path.value[k][j];
                }
                payoff = redemption[j-1]
                    / jointProcess->numeraire(grid[j], states);
                break;
            }
            else if (j == maturity) {
                Array states(5);
                for (Size k=0; k < 5; ++k) {
                    states[k] = path.value[k][j];
                }
                payoff = 1.0 / jointProcess->numeraire(grid[j], states);
            }
        }

        if (antithetic){
            stat.add(0.5*(antitheticPayoff + payoff));
        }
        else {
            antitheticPayoff = payoff;
        } 
    }

    const Real expected = 0.938;
    const Real calculated = stat.mean();
    const Real error = stat.errorEstimate();

    if (std::fabs(expected - calculated) > 3*error) {
        BOOST_ERROR("Failed to reproduce auto-callable equity structure price"
                    << "\n   calculated: " << calculated
                    << "\n   error:      " << error
                    << "\n   expected:   " << expected);        
    }
}



namespace {
    // Multi vanilla option instrument allows to price sevaral 
    // vanilla options using one Monte-Carlo simulation run.
    // Needed here to measure the calibration missmatch of all 
    // calibration instruments within one Monte-Carlo simulation.
    // Having one Monte-Carlo simulation for each calibration instrument
    // would take far too long.

     class MultiVanillaOption : public Instrument {
      public:
        class engine;
        MultiVanillaOption(
            const boost::shared_ptr<StochasticProcess> & process,
            const std::vector<boost::shared_ptr<StrikedTypePayoff> > & payoffs,
            const std::vector<boost::shared_ptr<Exercise> > & exercises,
            const boost::shared_ptr<PricingEngine>& engine =
                                 boost::shared_ptr<PricingEngine>());

        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results* results) const; 

        bool isExpired() const;
        const std::vector<Real> & NPVs() const;

        class arguments;
        class results;

      protected:
        const boost::shared_ptr<StochasticProcess>       stochasticProcess_;
        const std::vector<boost::shared_ptr<StrikedTypePayoff> >  payoffs_;
        const std::vector<boost::shared_ptr<Exercise> >           exercises_;

        mutable std::vector<Real> value_;
        mutable std::vector<Real> errorEstimate_;
    };
    
    class MultiVanillaOption::arguments 
        : public virtual PricingEngine::arguments {
      public:
        arguments() {}
        void validate() const;

        boost::shared_ptr<Exercise> exercise;
        boost::shared_ptr<StochasticProcess> stochasticProcess;
        std::vector<boost::shared_ptr<VanillaOption::arguments> > optionArgs;
    };

    class MultiVanillaOption::results : public virtual PricingEngine::results {
      public:
        results() {}
        void reset();
        
        std::vector<Real> value;
        std::vector<Real> errorEstimate;
    };

    class MultiVanillaOption::engine :
        public GenericEngine<MultiVanillaOption::arguments,
                             MultiVanillaOption::results> {};

    MultiVanillaOption::MultiVanillaOption(
         const boost::shared_ptr<StochasticProcess> & process,
         const std::vector<boost::shared_ptr<StrikedTypePayoff> > & payoffs,
         const std::vector<boost::shared_ptr<Exercise> > & exercises,
         const boost::shared_ptr<PricingEngine>& engine)
    : stochasticProcess_(process),
      payoffs_(payoffs),
      exercises_(exercises) {
        QL_REQUIRE(payoffs_.size() == exercises_.size(),
                   "number of payoffs and exercises must match");

        if (engine) {
            setPricingEngine(engine);
        }
    }

    void MultiVanillaOption::setupArguments(
                                     PricingEngine::arguments* args) const {
        MultiVanillaOption::arguments* arguments =
            dynamic_cast<MultiVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->stochasticProcess  = stochasticProcess_;
        const boost::shared_ptr<JointStochasticProcess> jointProcess = 
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                                                           stochasticProcess_);

        boost::shared_ptr<StochasticProcess> optionProcess;
        if (jointProcess) {
            optionProcess = jointProcess->constituents()[0];
        }
        else {
            optionProcess = stochasticProcess_;
        }
        arguments->optionArgs.resize(payoffs_.size());

        for (Size i=0; i < payoffs_.size(); ++i) {
            arguments->optionArgs[i] 
              = boost::shared_ptr<VanillaOption::arguments>(
                                              new VanillaOption::arguments());

            arguments->optionArgs[i]->payoff   = payoffs_[i];
            arguments->optionArgs[i]->exercise = exercises_[i];
            arguments->optionArgs[i]->stochasticProcess = optionProcess;
        }
        arguments->exercise = exercises_.back();
    }

    void MultiVanillaOption::fetchResults(
                                     const PricingEngine::results* r) const {
        const MultiVanillaOption::results* results =
            dynamic_cast<const MultiVanillaOption::results*>(r);
        QL_REQUIRE(results != 0,
                   "incorrect result type return from pricing engine");
        
        value_.resize(results->value.size());
        errorEstimate_.resize(results->value.size());

        std::copy(results->value.begin(), results->value.end(), 
                  value_.begin());
        std::copy(results->errorEstimate.begin(),
                  results->errorEstimate.end(), errorEstimate_.begin());
        NPV_ = value_[0];
    }

    bool MultiVanillaOption::isExpired() const {
        const Date evaluationDate = Settings::instance().evaluationDate();
        for (std::vector<boost::shared_ptr<Exercise> >::const_iterator 
                 iter = exercises_.begin(); iter != exercises_.end(); ++iter) {
            if ((*iter)->lastDate() >= evaluationDate) {
                return false;
            }
        }
        return true;
    }

    void MultiVanillaOption::arguments::validate() const {
        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator 
                 iter = optionArgs.begin(); iter != optionArgs.end(); ++iter) {
            (*iter)->validate();
        }
    }

    void MultiVanillaOption::results::reset() {
        std::fill(value.begin(), value.end(), Null<Real>());
        std::fill(errorEstimate.begin(), errorEstimate.end(), Null<Real>());
    }

    const std::vector<Real> & MultiVanillaOption::NPVs() const {
        NPV();
        Array t = Array(errorEstimate_)/Array(value_);
        return value_;
    }

    class MultiVanillaOptionEngine : public MultiVanillaOption::engine {
      public:
        MultiVanillaOptionEngine(
                           const boost::shared_ptr<PricingEngine> & engine);

        void calculate() const;
      private:
        const boost::shared_ptr<PricingEngine> engine_;
    };   


    MultiVanillaOptionEngine::MultiVanillaOptionEngine(
                             const boost::shared_ptr<PricingEngine> & engine) 
    : engine_(engine) {
    }

    void MultiVanillaOptionEngine::calculate() const {
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > & 
            optionArgs = arguments_.optionArgs;

        results_.value.resize(optionArgs.size());
        results_.errorEstimate.resize(optionArgs.size());

        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter=optionArgs.begin(); iter !=optionArgs.end(); ++iter) {

            const boost::shared_ptr<StrikedTypePayoff> payoff =
               boost::dynamic_pointer_cast<StrikedTypePayoff>((*iter)->payoff);

            const boost::shared_ptr<Exercise> exercise = (*iter)->exercise;

            const VanillaOption option((*iter)->stochasticProcess, 
                                       payoff, exercise, engine_);

            const Size i = iter - optionArgs.begin();

            results_.errorEstimate[i] = 0;
            results_.value[i] = option.NPV();
        }
    }

    class MultiEuropeanPathPricer : public PathPricer<MultiPath, Array> {
      public:
        MultiEuropeanPathPricer(
            const Size stateIndex,
            const TimeGrid& timeGrid,
            const boost::shared_ptr<JointStochasticProcess> & process,
            const std::vector<boost::shared_ptr<VanillaOption::arguments> > &,
            const boost::shared_ptr<YieldTermStructure> & termStructure 
                                  = boost::shared_ptr<YieldTermStructure>());

        Array operator()(const MultiPath& path) const;
      private:
        const Size stateIndex_;
        const boost::shared_ptr<JointStochasticProcess> process_;
        const boost::shared_ptr<YieldTermStructure> termStructure_;

        std::vector<Time> exerciseTimes_;
        std::vector<Size> exerciseIndices_;
        std::vector<boost::shared_ptr<Payoff> > payoffs_;
    };

    MultiEuropeanPathPricer::MultiEuropeanPathPricer(
        const Size stateIndex,
        const TimeGrid& timeGrid,
        const boost::shared_ptr<JointStochasticProcess> & process,
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > & args,
        const boost::shared_ptr<YieldTermStructure> & termStructure)
    : stateIndex_(stateIndex),
      process_(process),
      termStructure_(termStructure) {
        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter = args.begin(); iter != args.end(); ++iter) {

            boost::shared_ptr<Exercise> exercise = (*iter)->exercise;
            QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");
            const Time exerciseTime = process_->time(exercise->lastDate());

            payoffs_.push_back((*iter)->payoff);
            exerciseTimes_.push_back(exerciseTime);
            exerciseIndices_.push_back(timeGrid.index(exerciseTime));
        }
    }
 
    Array MultiEuropeanPathPricer::operator()(const MultiPath& path) const {
        QL_REQUIRE(path.pathSize() > 0, "the path cannot be empty");

        Array retVal(payoffs_.size());
        for (Size i=0; i < payoffs_.size(); ++i) {
            const Time t = exerciseTimes_[i];
            const Size index = exerciseIndices_[i];

            Array states(path.assetNumber());
            for (Size j=0; j < states.size(); ++j) {
                states[j] = path[j][index];
            }
            const Real state = states[stateIndex_];
            const DiscountFactor df 
                = (termStructure_) ? termStructure_->discount(t)
                                   : 1.0/process_->numeraire(t, states);

            retVal[i] = (*payoffs_[i])(state) * df;
        }

        return retVal;
    }


    //! default Monte Carlo traits for multi-variate models
    template <class RNG = PseudoRandom>
    struct MultiVariateMultiPricer {
        typedef RNG rng_traits;
        typedef MultiPath path_type;
        typedef PathPricer<path_type, Array> path_pricer_type;
        typedef typename RNG::rsg_type rsg_type;
        typedef MultiPathGenerator<rsg_type> path_generator_type;
        enum { allowsErrorEstimate = RNG::allowsErrorEstimate };
    };
    template<class RNG,class S = Statistics>
    class MCEuropeanMultiEngine 
        : public MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                 GenericSequenceStatistics<S>, 
                                 MultiVanillaOption> {

      public:
        typedef MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                GenericSequenceStatistics<S>, 
                                MultiVanillaOption> base_type;
        typedef typename base_type::path_generator_type path_generator_type;
        typedef typename base_type::path_pricer_type path_pricer_type;
        typedef typename base_type::stats_type stats_type;
        typedef typename base_type::result_type result_type;

        MCEuropeanMultiEngine(Size timeSteps,
                              Size timeStepsPerYear,
                              bool antitheticVariate,
                              bool controlVariate,
                              Size requiredSamples,
                              Real requiredTolerance,
                              Size maxSamples,
                              BigNatural seed);

      protected:
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;    
    };


    template <class RNG, class S = Statistics>
    class MCMultiEuropeanHestonEngine 
        : public MCEuropeanMultiEngine<RNG,S> {
      public:
        typedef MCEuropeanMultiEngine<RNG,S> base_type;
        typedef typename base_type::path_generator_type path_generator_type;
        typedef typename base_type::path_pricer_type path_pricer_type;
        typedef typename base_type::stats_type stats_type;
        typedef typename base_type::result_type result_type;

        MCMultiEuropeanHestonEngine(Size timeSteps,
                                    Size timeStepsPerYear,
                                    bool antitheticVariate,
                                    bool controlVariate,
                                    Size requiredSamples,
                                    Real requiredTolerance,
                                    Size maxSamples,
                                    BigNatural seed);

      protected:
        boost::shared_ptr<path_pricer_type> controlPathPricer() const;
        boost::shared_ptr<PricingEngine> controlPricingEngine() const;
    };



    template<class RNG,class S>
    inline MCEuropeanMultiEngine<RNG,S>::MCEuropeanMultiEngine(
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
        : base_type(timeSteps, timeStepsPerYear,
                      false,antitheticVariate,
                      controlVariate, requiredSamples,
                      requiredTolerance, maxSamples, seed) {
    }

    
    template <class RNG,class S>
    inline TimeGrid MCEuropeanMultiEngine<RNG,S>::timeGrid() const {

        // first get time steps from super class
        const TimeGrid tmpGrid
            = this->MCVanillaEngine<MultiVariateMultiPricer,RNG,
                                    GenericSequenceStatistics<S>, 
                                    MultiVanillaOption>::timeGrid();
        
        // add option expiry as mandatory dates
        const std::vector<boost::shared_ptr<VanillaOption::arguments> > & 
            optionArgs = this->arguments_.optionArgs;

        std::vector<Time> tmpTimes(tmpGrid.begin(), tmpGrid.end());

        for (std::vector<
                 boost::shared_ptr<VanillaOption::arguments> >::const_iterator
                 iter = optionArgs.begin(); iter != optionArgs.end(); ++iter) {
            const std::vector<Date> & dates = (*iter)->exercise->dates();
            for (std::vector<Date>::const_iterator iter = dates.begin();
                 iter != dates.end(); ++iter) {
                tmpTimes.push_back(
                          this->arguments_.stochasticProcess->time(*iter));
            }
        }
        
        // add libor fixing dates as mandatory dates
        boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

        return TimeGrid(tmpTimes.begin(), tmpTimes.end());
    }

    template <class RNG,class S> inline 
    boost::shared_ptr<typename MCEuropeanMultiEngine<RNG,S>::path_pricer_type>
    MCEuropeanMultiEngine<RNG,S>::pathPricer() const {
       boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       return boost::shared_ptr<path_pricer_type>(
            new MultiEuropeanPathPricer(0, timeGrid(), process, 
                                        this->arguments_.optionArgs));
    }

    template<class RNG,class S>
    inline MCMultiEuropeanHestonEngine<RNG,S>::MCMultiEuropeanHestonEngine(
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
        : base_type(timeSteps, timeStepsPerYear,
                    antitheticVariate, controlVariate, 
                    requiredSamples, requiredTolerance, 
                    maxSamples, seed) {
    }

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCMultiEuropeanHestonEngine<RNG,S>::path_pricer_type> 
    MCMultiEuropeanHestonEngine<RNG,S>::controlPathPricer() const {
        boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       boost::shared_ptr<HestonProcess> hestonProcess = 
           boost::dynamic_pointer_cast<HestonProcess>(
                                                 process->constituents()[0]);

       QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic " 
                                 "process need to be of type HestonProcess");

       return boost::shared_ptr<path_pricer_type>(
            new MultiEuropeanPathPricer(2,
                    this->timeGrid(), process, 
                    this->arguments_.optionArgs,
                    hestonProcess->riskFreeRate().currentLink()));
    }
    
    template <class RNG, class S> inline
    boost::shared_ptr<PricingEngine> 
    MCMultiEuropeanHestonEngine<RNG,S>::controlPricingEngine() const {
        boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       boost::shared_ptr<HestonProcess> hestonProcess = 
           boost::dynamic_pointer_cast<HestonProcess>(
                                                 process->constituents()[0]);

       QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic " 
                                 "process need to be of type HestonProcess");
       
       boost::shared_ptr<HestonModel> model(new HestonModel(hestonProcess));


       boost::shared_ptr<PricingEngine> retVal(
           new MultiVanillaOptionEngine(boost::shared_ptr<PricingEngine>(
                                     new AnalyticHestonEngine(model, 192))));
       return retVal;
    }

    class KappaConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            Impl(Real lower, Real upper) 
                : lower_(lower), upper_(upper) {}
            bool test(const Array& params) const {
                const Real kappa = params[1];

                return (kappa > lower_ && kappa < upper_);
            }
          private:
            const Real lower_, upper_;
        };
      public:
        KappaConstraint(Real lower, Real upper)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                              new KappaConstraint::Impl(lower, upper))) {}
    };
}

void HybridHestonHullWhiteProcessTest::testPseudoJointCalibration() {
    BOOST_MESSAGE("Testing Joint Calibration of an Heston Equity Process"
                  "incl. Stochastic Interest Rates via a Hull-White Model...");

    SavedSettings backup;

    Calendar calendar = TARGET();
    DayCounter dc = Actual365Fixed();
    const Date today = Date::todaysDate();

    Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.04));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<StochasticProcess> bsmProcess(
               new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    const boost::shared_ptr<HestonProcess> hestonProcess(
                          new HestonProcess(rTS, qTS, spot, 0.0625, 
                                            1.0, 0.0625, 0.01, 0.0));
    const boost::shared_ptr<HestonModel> hestonModel(
                          new HestonModel(hestonProcess));

    const boost::shared_ptr<HullWhiteForwardProcess> hwProcess(
                          new HullWhiteForwardProcess(rTS, 0.012, 0.01));
    hwProcess->setForwardMeasureTime(
                      dc.yearFraction(today, today+Period(11, Years)));

    const Real corr = -0.3;
    const boost::shared_ptr<JointStochasticProcess> jointProcess(
        new HybridHestonHullWhiteProcess(hestonProcess, hwProcess, corr, 5));
    
    std::vector<Period> optionMaturities;
    optionMaturities.push_back(Period(1, Years));
    optionMaturities.push_back(Period(2, Years));    
    optionMaturities.push_back(Period(5, Years));
    optionMaturities.push_back(Period(10, Years));

    Real strikes[] = { 50, 66.6, 80, 100, 125, 150, 200 };

    std::vector<Volatility> impliedVols;
    std::vector<Real> myNPVs;
    std::vector<RelinkableHandle<Quote> > hestonVols;
    std::vector<boost::shared_ptr<Exercise> > exercises;
    std::vector<boost::shared_ptr<StrikedTypePayoff> > payoffs;

    std::vector<boost::shared_ptr<EuropeanOption> > bsmOptions;
    std::vector<boost::shared_ptr<CalibrationHelper> > hestonOptions;

    boost::shared_ptr<HullWhite> hullWhiteModel(
        new HullWhite(Handle<YieldTermStructure>(rTS), 
                      hwProcess->a(), hwProcess->sigma()));

    boost::shared_ptr<PricingEngine> bsmhwEngine(
                        new AnalyticBSMHullWhiteEngine(corr, hullWhiteModel));

    for (Size i=0; i < optionMaturities.size(); ++i) {
        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Date maturityDate 
                = calendar.advance(today, optionMaturities[i]);
            const Time t = dc.yearFraction(today, maturityDate);
            const Real fwd = spot->value()/rTS->discount(t)*qTS->discount(t);

            const Real gf = std::exp(4.0*vol->value()*std::sqrt(t));

            // use 4-sigma options only
            if (strikes[j] < fwd/gf || strikes[j] > fwd*gf) {
                continue;
            }

            // calculate market value and
            // "implied" volatility of a BSM-Hull-White option
            exercises.push_back(boost::shared_ptr<Exercise>(
                                        new EuropeanExercise(maturityDate)));

            payoffs.push_back(boost::shared_ptr<StrikedTypePayoff>(
                          new PlainVanillaPayoff(Option::Call, strikes[j])));

            const Real npv =
                EuropeanOption(bsmProcess, payoffs.back(),
                               exercises.back(), bsmhwEngine).NPV();
            myNPVs.push_back(npv);
            
            bsmOptions.push_back(boost::shared_ptr<EuropeanOption>(
                new EuropeanOption(bsmProcess, 
                                   payoffs.back(),exercises.back())));

            impliedVols.push_back(
                bsmOptions.back()->impliedVolatility(npv, 1e-10, 100));

            hestonVols.push_back(RelinkableHandle<Quote>(
              boost::shared_ptr<Quote>(new SimpleQuote(impliedVols.back()))));

            hestonOptions.push_back(boost::shared_ptr<CalibrationHelper>(
                  new HestonModelHelper(optionMaturities[i], calendar,
                                        spot->value(), strikes[j], 
                                        hestonVols.back(), rTS, qTS, false)));
        }
    }

    // cascade joint calibration
    boost::shared_ptr<PricingEngine> engine(
                                 new AnalyticHestonEngine(hestonModel, 192));

    for (std::vector<boost::shared_ptr<CalibrationHelper> >::const_iterator
           iter = hestonOptions.begin(); iter != hestonOptions.end(); ++iter) {
        (*iter)->setPricingEngine(engine);
    }

    boost::shared_ptr<MCEuropeanMultiEngine<PseudoRandom> > mcHestonEngine(
        new MCMultiEuropeanHestonEngine<PseudoRandom >(
             1, Null<Size>(), true, true, 1, 0.1, Null<Size>(), 123));
    MultiVanillaOption mvo(jointProcess, payoffs, exercises, 
                           mcHestonEngine);
    mvo.registerWith(hestonModel);

    Real qualityIndex;
    const Size nrCascadeSteps = 2;
    for (Size i=0; i<nrCascadeSteps; ++i) {
        // 1. Calibrate Heston Model to match 
        //    current Heston Volatility surface
        LevenbergMarquardt lm(1e-8, 1e-8, 1e-8);
        hestonModel->calibrate(hestonOptions, lm, 
                               EndCriteria(400, 100, 1.0e-8, 1.0e-8, 1.0e-8));

        // 2. Calculate NPVs under the full model
        const std::vector<Real> npvs = mvo.NPVs();

        // 3. calculate vola implied vols
        std::vector<Volatility> diffVols(npvs.size());

        Real sse = 0.0;
        for (Size i=0; i < npvs.size(); ++i) {
            diffVols[i] = bsmOptions[i]->impliedVolatility(npvs[i], 1e-10) 
                         - impliedVols[i];

            sse += (npvs[i] - myNPVs[i])*(npvs[i] - myNPVs[i]);
        }
        qualityIndex = 100*std::sqrt(sse/hestonOptions.size());

        // 4. correct heston option vol by diffVol to correct the plain
        //    the plain vanilla calibration
        for (Size i=0; i < npvs.size(); ++i) {
            const Volatility currVol = hestonVols[i].currentLink()->value();

            hestonVols[i].linkTo(boost::shared_ptr<Quote>(
                  new SimpleQuote(std::max(currVol - diffVols[i], 0.0011))));
        }
    }

    if (qualityIndex > 5.0) {
        BOOST_ERROR("Failed to calibrate Heston Hull-White Model");
    }
}

test_suite* HybridHestonHullWhiteProcessTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Hybrid Heston-HullWhite tests");

    suite->add(BOOST_TEST_CASE(
            &HybridHestonHullWhiteProcessTest::testBsmHullWhiteEngine));
    suite->add(BOOST_TEST_CASE(
            &HybridHestonHullWhiteProcessTest::testZeroBondPricing));
    suite->add(BOOST_TEST_CASE(
            &HybridHestonHullWhiteProcessTest::testMcVanillaPricing));
    suite->add(BOOST_TEST_CASE(
            &HybridHestonHullWhiteProcessTest::testCallableEquityPricing));
    suite->add(BOOST_TEST_CASE(
            &HybridHestonHullWhiteProcessTest::testPseudoJointCalibration));

    return suite;
}

