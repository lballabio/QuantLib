/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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
#include "squarerootclvmodel.hpp"
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/instruments/forwardvanillaoption.hpp>
#include <ql/math/statistics/statistics.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/processes/squarerootprocess.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/forward/forwardengine.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/noexceptlocalvolsurface.hpp>
#include <ql/experimental/models/squarerootclvmodel.hpp>
#include <ql/models/equity/hestonslvfdmmodel.hpp>
#include <ql/processes/hestonslvprocess.hpp>
#include <ql/pricingengines/barrier/fdhestondoublebarrierengine.hpp>
#include <ql/pricingengines/barrier/analyticdoublebarrierbinaryengine.hpp>
#include <ql/experimental/volatility/sabrvoltermstructure.hpp>

#include <boost/math/distributions/non_central_chi_squared.hpp>

#include <set>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;


namespace square_root_clv_model {
    class CLVModelPayoff : public PlainVanillaPayoff {
      public:
        CLVModelPayoff(Option::Type type, Real strike, ext::function<Real(Real)> g)
        : PlainVanillaPayoff(type, strike), g_(std::move(g)) {}

        Real operator()(Real x) const override { return PlainVanillaPayoff::operator()(g_(x)); }

      private:
        const ext::function<Real(Real)> g_;
    };

    typedef boost::math::non_central_chi_squared_distribution<Real>
        chi_squared_type;
}


void SquareRootCLVModelTest::testSquareRootCLVVanillaPricing() {
    BOOST_TEST_MESSAGE(
        "Testing vanilla option pricing with square-root kernel process...");

    using namespace square_root_clv_model;

    SavedSettings backup;

    const Date todaysDate(5, Oct, 2016);
    Settings::instance().evaluationDate() = todaysDate;

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date maturityDate = todaysDate + Period(3, Months);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));

    const Rate r = 0.08;
    const Rate q = 0.03;
    const Volatility vol = 0.3;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(todaysDate, vol, dc));
    const Real fwd = s0*qTS->discount(maturity)/rTS->discount(maturity);

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        ext::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, volTS));

    const Real kappa       = 1.0;
    const Real theta       = 0.06;
    const Volatility sigma = 0.2;
    const Real x0          = 0.09;

    const ext::shared_ptr<SquareRootProcess> sqrtProcess(
        ext::make_shared<SquareRootProcess>(theta, kappa, sigma, x0));

    const std::vector<Date> maturityDates(1, maturityDate);

    const SquareRootCLVModel model(
        bsProcess, sqrtProcess, maturityDates, 14, 1-1e-14, 1e-14);

    const Array x = model.collocationPointsX(maturityDate);
    const Array y = model.collocationPointsY(maturityDate);

    const LagrangeInterpolation g(x.begin(), x.end(), y.begin());

    const Real df  = 4*theta*kappa/(sigma*sigma);
    const Real ncp = 4*kappa*std::exp(-kappa*maturity)
            / (sigma*sigma*(1-std::exp(-kappa*maturity)))*sqrtProcess->x0();

    const chi_squared_type dist(df, ncp);
        
    const Real strikes[] = { 50, 75, 100, 125, 150, 200 };
    for (Real strike : strikes) {
        const Option::Type optionType = (strike > fwd) ? Option::Call : Option::Put;

        const Real expected = BlackCalculator(
            optionType, strike, fwd,
            std::sqrt(volTS->blackVariance(maturity, strike)),
            rTS->discount(maturity)).value();

        const CLVModelPayoff clvModelPayoff(optionType, strike, g);

        const ext::function<Real(Real)> f = [&](Real xi) -> Real {
            return clvModelPayoff(xi) * boost::math::pdf(dist, xi);
        };

        const Real calculated = GaussLobattoIntegral(1000, 1e-6)(
            f, x.front(), x.back()) * rTS->discount(maturity);

        const Real tol = 5e-3;
        if (std::fabs(expected - calculated) > tol) {
            BOOST_FAIL("failed to reproduce option SquaredCLVMOdel prices"
                    << "\n    time:       " << maturityDate
                    << "\n    strike:     " << strike
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated);
        }
    }
}

void SquareRootCLVModelTest::testSquareRootCLVMappingFunction() {
    BOOST_TEST_MESSAGE(
        "Testing mapping function of the square-root kernel process...");

    using namespace square_root_clv_model;

    SavedSettings backup;

    const Date todaysDate(16, Oct, 2016);
    Settings::instance().evaluationDate() = todaysDate;
    const Date maturityDate = todaysDate + Period(1, Years);

    const DayCounter dc = Actual365Fixed();

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));

    const Rate r = 0.05;
    const Rate q = 0.02;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    //SABR
    const Real beta =  0.95;
    const Real alpha=  0.2;
    const Real rho  = -0.9;
    const Real gamma=  0.8;

    const Handle<BlackVolTermStructure> sabrVol(
        ext::make_shared<SABRVolTermStructure>(
            alpha, beta, gamma, rho, s0, r, todaysDate, dc));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        ext::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, sabrVol));

    std::vector<Date> calibrationDates(1, todaysDate + Period(3, Months));
    calibrationDates.reserve(Size(daysBetween(todaysDate, maturityDate)/7 + 1));
    while (calibrationDates.back() < maturityDate)
        calibrationDates.push_back(calibrationDates.back() + Period(1, Weeks));

    // sqrt process
    const Real kappa       = 1.0;
    const Real theta       = 0.09;
    const Volatility sigma = 0.2;
    const Real x0          = 0.09;

    const ext::shared_ptr<SquareRootProcess> sqrtProcess(
        ext::make_shared<SquareRootProcess>(theta, kappa, sigma, x0));

    const SquareRootCLVModel model(
        bsProcess, sqrtProcess, calibrationDates, 14, 1-1e-10, 1e-10);

    const ext::function<Real(Time, Real)> g = model.g();

    const Real strikes[] = { 80, 100, 120 };
    const Size offsets[] = { 92, 182, 183, 184, 185, 186, 365 };
    for (unsigned long offset : offsets) {
        const Date m = todaysDate + Period(offset, Days);
        const Time t = dc.yearFraction(todaysDate, m);

        const Real df  = 4*theta*kappa/(sigma*sigma);
        const Real ncp = 4*kappa*std::exp(-kappa*t)
                / (sigma*sigma*(1-std::exp(-kappa*t)))*sqrtProcess->x0();

        const chi_squared_type dist(df, ncp);

        const Real fwd = s0*qTS->discount(m)/rTS->discount(m);

        for (Real strike : strikes) {
            const Option::Type optionType = (strike > fwd) ? Option::Call : Option::Put;

            const Real expected = BlackCalculator(
                optionType, strike, fwd,
                std::sqrt(sabrVol->blackVariance(m, strike)),
                rTS->discount(m)).value();

            const CLVModelPayoff clvModelPayoff(optionType, strike, [&](Real x) { return g(t, x); });

            const ext::function<Real(Real)> f = [&](Real xi) -> Real {
                return clvModelPayoff(xi) * boost::math::pdf(dist, xi);
            };

            const Array x = model.collocationPointsX(m);
            const Real calculated = GaussLobattoIntegral(1000, 1e-3)(
                f, x.front(), x.back()) * rTS->discount(m);

            const Real tol = 0.075;

            if (std::fabs(expected) > 0.01
                    && std::fabs((calculated - expected)/calculated) > tol) {
                BOOST_FAIL("failed to reproduce option SquaredCLVMOdel prices"
                        << "\n    time:       " << m
                        << "\n    strike:     " << strike
                        << "\n    expected:   " << expected
                        << "\n    calculated: " << calculated);
            }
        }
    }
}

namespace square_root_clv_model {
    class SquareRootCLVCalibrationFunction : public CostFunction {
      public:
        SquareRootCLVCalibrationFunction(Array strikes,
                                         const std::vector<Date>& resetDates,
                                         const std::vector<Date>& maturityDates,
                                         ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
                                         Array refVols,
                                         Size nScenarios = 10000)
        : strikes_(std::move(strikes)), resetDates_(resetDates), maturityDates_(maturityDates),
          bsProcess_(std::move(bsProcess)), refVols_(std::move(refVols)), nScenarios_(nScenarios) {
            std::set<Date> c(resetDates.begin(), resetDates.end());
            c.insert(maturityDates.begin(), maturityDates.end());
            calibrationDates_.insert(
                calibrationDates_.begin(), c.begin(), c.end());
        }

        Real value(const Array& params) const override {
            const Array diff = values(params);

            Real retVal = 0.0;
            for (Real i : diff)
                retVal += i * i;

            return retVal;
        }

        Array values(const Array& params) const override {
            const Real theta = params[0];
            const Real kappa = params[1];
            const Real sigma = params[2];
            const Real x0    = params[3];

            const ext::shared_ptr<SimpleQuote> vol(
                ext::make_shared<SimpleQuote>(0.1));

            const Handle<YieldTermStructure> rTS(bsProcess_->riskFreeRate());
            const Handle<YieldTermStructure> qTS(bsProcess_->dividendYield());
            const Handle<Quote> spot(ext::make_shared<SimpleQuote>(
                bsProcess_->x0()));

            const ext::shared_ptr<PricingEngine> fwdEngine(
                ext::make_shared<ForwardVanillaEngine<AnalyticEuropeanEngine> >(
                    ext::make_shared<GeneralizedBlackScholesProcess>(
                        spot, qTS, rTS,
                        Handle<BlackVolTermStructure>(
                            flatVol(rTS->referenceDate(), vol,
                                    rTS->dayCounter())))));

            const ext::shared_ptr<SquareRootProcess> sqrtProcess(
                ext::make_shared<SquareRootProcess>(theta, kappa, sigma, x0));

            const SquareRootCLVModel clvSqrtModel(
                bsProcess_, sqrtProcess, calibrationDates_,
                14, 1-1e-14, 1e-14);

            const ext::function<Real(Time, Real)> gSqrt = clvSqrtModel.g();

            Array retVal(resetDates_.size()*strikes_.size());

            for (Size i=0, n=resetDates_.size(); i < n; ++i) {
                const Date resetDate = resetDates_[i];
                const Date maturityDate = maturityDates_[i];

                const Time t0 = bsProcess_->time(resetDate);
                const Time t1 = bsProcess_->time(maturityDate);

                const Real df  = 4*theta*kappa/(sigma*sigma);
                const Real ncp = 4*kappa*std::exp(-kappa*t0)
                    / (sigma*sigma*(1-std::exp(-kappa*t0)))*x0;

                typedef boost::math::non_central_chi_squared_distribution<Real>
                    chi_squared_type;

                const chi_squared_type dist(df, ncp);

                const Real ncp1 = 4*kappa*std::exp(-kappa*(t1-t0))
                    / (sigma*sigma*(1-std::exp(-kappa*(t1-t0))));

                const LowDiscrepancy::ursg_type ursg = LowDiscrepancy::ursg_type(2, 1235UL);

                std::vector<GeneralStatistics> stats(strikes_.size());

                for (Size j=0; j < nScenarios_; ++j) {
                    const std::vector<Real>& path = ursg.nextSequence().value;

                    const Real x1 = boost::math::quantile(dist, path[0]);
                    const Real u1 =
                        sigma*sigma*(1-std::exp(-kappa*t0))/(4*kappa)*x1;

                    const Real x2 = boost::math::quantile(
                        chi_squared_type(df, ncp1*u1), path[1]);
                    const Real u2 =
                        sigma*sigma*(1-std::exp(-kappa*(t1-t0)))/(4*kappa)*x2;
                    const Real X2 =
                        u2*4*kappa/(sigma*sigma*(1-std::exp(-kappa*t1)));

                    const Real s1 = gSqrt(t0, x1);
                    const Real s2 = gSqrt(t1, X2);

                    for (Size k=0; k < strikes_.size(); ++k) {
                        const Real strike = strikes_[k];

                        const Real payoff = (strike < 1.0)
                            ?  Real(s1 * std::max(0.0, strike - s2/s1))
                            :  Real(s1 * std::max(0.0, s2/s1 - strike));

                        stats[k].add(payoff);
                    }
                }

                const ext::shared_ptr<Exercise> exercise(
                    ext::make_shared<EuropeanExercise>(maturityDate));

                const DiscountFactor dF(
                    bsProcess_->riskFreeRate()->discount(maturityDate));

                for (Size k=0; k < strikes_.size(); ++k) {
                    const Real strike = strikes_[k];
                    const Real npv = stats[k].mean() * dF;

                    const ext::shared_ptr<StrikedTypePayoff> payoff(
                        ext::make_shared<PlainVanillaPayoff>(
                            (strike < 1.0) ? Option::Put : Option::Call, strike));

                    const ext::shared_ptr<ForwardVanillaOption> fwdOption(
                        ext::make_shared<ForwardVanillaOption>(
                            strike, resetDate, payoff, exercise));

                    const Volatility implVol =
                        QuantLib::detail::ImpliedVolatilityHelper::calculate(
                            *fwdOption, *fwdEngine, *vol, npv, 1e-8, 200, 1e-4, 2.0);

                    const Size idx = k + i*strikes_.size();
                    retVal[idx] = implVol - refVols_[idx];
                }
            }

            return retVal;
        }


      private:
        const Array strikes_;
        const std::vector<Date> resetDates_, maturityDates_;
        const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        const Array refVols_;
        const Size nScenarios_;

        std::vector<Date> calibrationDates_;
    };

    class NonZeroConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            bool test(const Array& params) const override {
                const Real theta = params[0];
                const Real kappa = params[1];
                const Real sigma = params[2];
                const Real x0    = params[3];

                return (sigma >= 0.001 && kappa > 1e-6 && theta > 0.001
                        && x0 > 1e-4);
            }

            Array upperBound(const Array& params) const override {
                const Real upper[] = { 1.0, 1.0, 1.0, 2.0 };

                return Array(upper, upper + 4);
            }

            Array lowerBound(const Array& params) const override {
                const Real lower[] = { 0.001, 0.001, 0.001, 1e-4 };

                return Array(lower, lower + 4);
            }
        };

      public:
        NonZeroConstraint()
        : Constraint(ext::make_shared<NonZeroConstraint::Impl>()) {}
    };
}

void SquareRootCLVModelTest::testForwardSkew() {
    BOOST_TEST_MESSAGE(
        "Testing forward skew dynamics with square-root kernel process...");

    using namespace square_root_clv_model;

    SavedSettings backup;

    const Date todaysDate(16, Oct, 2016);
    Settings::instance().evaluationDate() = todaysDate;
    const Date endDate = todaysDate + Period(4, Years);

    const DayCounter dc = Actual365Fixed();

    // Heston model is used to generate an arbitrage free volatility surface
    const Real s0    =  100;
    const Real r     =  0.1;
    const Real q     =  0.05;
    const Real v0    =  0.09;
    const Real kappa =  1.0;
    const Real theta =  0.09;
    const Real sigma =  0.3;
    const Real rho   = -0.75;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, sigma, rho)));

    const Handle<BlackVolTermStructure> blackVol(
        ext::make_shared<HestonBlackVolSurface>(
            Handle<HestonModel>(hestonModel)));

    const Handle<LocalVolTermStructure> localVol(
        ext::make_shared<NoExceptLocalVolSurface>(
                blackVol, rTS, qTS, spot, std::sqrt(theta)));

    const Real sTheta = 0.389302;
    const Real sKappa = 0.1101849;
    const Real sSigma = 0.275368;
    const Real sX0    = 0.466809;

    const ext::shared_ptr<SquareRootProcess> sqrtProcess(
        ext::make_shared<SquareRootProcess>(
            sTheta, sKappa, sSigma, sX0));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
        ext::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, blackVol));

    std::vector<Date> calibrationDates(1, todaysDate + Period(6, Months));
    while (calibrationDates.back() < endDate)
        calibrationDates.push_back(calibrationDates.back() + Period(3, Months));

    std::set<Date> clvCalibrationDates(
        calibrationDates.begin(), calibrationDates.end());

    Date tmpDate = todaysDate + Period(1, Days);
    while (tmpDate < todaysDate + Period(1, Years)) {
        clvCalibrationDates.insert(tmpDate);
        tmpDate += Period(1, Weeks);
    }

    const SquareRootCLVModel clvSqrtModel(
        bsProcess,
        sqrtProcess,
        std::vector<Date>(
            clvCalibrationDates.begin(), clvCalibrationDates.end()),
        14, 1-1e-14, 1e-14);

    const ext::function<Real(Time, Real)> gSqrt = clvSqrtModel.g();

    const ext::shared_ptr<SimpleQuote> vol(
        ext::make_shared<SimpleQuote>(0.1));

    const ext::shared_ptr<PricingEngine> fwdEngine(
        ext::make_shared<ForwardVanillaEngine<AnalyticEuropeanEngine> >(
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(flatVol(todaysDate, vol, dc)))));


    // forward skew of the Heston-SLV model
    std::vector<Time> mandatoryTimes;
    mandatoryTimes.reserve(calibrationDates.size());
    for (auto& calibrationDate : calibrationDates)
        mandatoryTimes.push_back(dc.yearFraction(todaysDate, calibrationDate));

    const Size tSteps = 200;
    const TimeGrid grid(mandatoryTimes.begin(), mandatoryTimes.end(), tSteps);

    std::vector<Date> resetDates, maturityDates;
    std::vector<Size> resetIndices, maturityIndices;
    for (Size i=0, n = calibrationDates.size()-2; i < n; ++i) {
        resetDates.push_back(calibrationDates[i]);
        maturityDates.push_back(calibrationDates[i+2]);

        const Time resetTime    = mandatoryTimes[i];
        const Time maturityTime = mandatoryTimes[i+2];

        resetIndices.push_back(grid.closestIndex(resetTime)-1);
        maturityIndices.push_back(grid.closestIndex(maturityTime)-1);
    }

    const Real strikes[] = {
        0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2,
        1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0
    };

    const Size nScenarios = 20000;
    Array refVols(resetIndices.size()*LENGTH(strikes));

    // finite difference calibration of Heston SLV model

    // define Heston Stochastic Local Volatility model
    const Real eta = 0.25;
    const Real corr = -0.0;

    const ext::shared_ptr<HestonProcess> hestonProcess4slv(
        ext::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, eta*sigma, corr));

    const Handle<HestonModel> hestonModel4slv(
        ext::make_shared<HestonModel>(hestonProcess4slv));

    const HestonSLVFokkerPlanckFdmParams logParams = {
        301, 601, 1000, 30, 2.0, 0, 2,
        0.1, 1e-4, 10000,
        1e-5, 1e-5, 0.0000025, 1.0, 0.1, 0.9, 1e-5,
        FdmHestonGreensFct::Gaussian,
        FdmSquareRootFwdOp::Log,
        FdmSchemeDesc::ModifiedCraigSneyd()
    };

    const ext::shared_ptr<LocalVolTermStructure> leverageFctFDM =
        HestonSLVFDMModel(localVol, hestonModel4slv, endDate, logParams).
            leverageFunction();

    //  calibrating to forward volatility dynamics

    const ext::shared_ptr<HestonSLVProcess> fdmSlvProcess(
        ext::make_shared<HestonSLVProcess>(
            hestonProcess4slv, leverageFctFDM));

    std::vector<std::vector<GeneralStatistics> > slvStats(
        calibrationDates.size()-2,
            std::vector<GeneralStatistics>(LENGTH(strikes)));

    typedef SobolBrownianBridgeRsg rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    const Size factors = fdmSlvProcess->factors();

    const ext::shared_ptr<MultiPathGenerator<rsg_type> > pathGen(
        ext::make_shared<MultiPathGenerator<rsg_type> >(
            fdmSlvProcess, grid, rsg_type(factors, grid.size()-1), false));

    for (Size k=0; k < nScenarios; ++k) {
        const sample_type& path = pathGen->next();

        for (Size i=0, n=resetIndices.size(); i < n; ++i) {
            const Real S_t1 = path.value[0][resetIndices[i]];
            const Real S_T1 = path.value[0][maturityIndices[i]];

            for (Size j=0; j < LENGTH(strikes); ++j) {
                const Real strike = strikes[j];
                    slvStats[i][j].add((strike < 1.0)
                        ? Real(S_t1 * std::max(0.0, strike - S_T1/S_t1))
                        : Real(S_t1 * std::max(0.0, S_T1/S_t1 - strike)));
            }

        }
    }

    for (Size i=0, n=resetIndices.size(); i < n; ++i) {
        const Date resetDate = calibrationDates[i];
        const Date maturityDate(calibrationDates[i+2]);
        const DiscountFactor df = rTS->discount(maturityDate);

        const ext::shared_ptr<Exercise> exercise(
            ext::make_shared<EuropeanExercise>(maturityDate));

        for (Size j=0; j < LENGTH(strikes); ++j) {
            const Real strike = strikes[j];
            const Real npv = slvStats[i][j].mean()*df;

            const ext::shared_ptr<StrikedTypePayoff> payoff(
                ext::make_shared<PlainVanillaPayoff>(
                    (strike < 1.0) ? Option::Put : Option::Call, strike));

            const ext::shared_ptr<ForwardVanillaOption> fwdOption(
                ext::make_shared<ForwardVanillaOption>(
                    strike, resetDate, payoff, exercise));

            const Volatility implVol =
                QuantLib::detail::ImpliedVolatilityHelper::calculate(
                    *fwdOption, *fwdEngine, *vol, npv, 1e-8, 200, 1e-4, 2.0);

            const Size idx = j + i*LENGTH(strikes);
            refVols[idx] = implVol;
        }
    }

    SquareRootCLVCalibrationFunction costFunction(
        Array(strikes, strikes+LENGTH(strikes)),
        resetDates,
        maturityDates,
        bsProcess,
        refVols,
        nScenarios);

    NonZeroConstraint nonZeroConstraint;

    CompositeConstraint constraint(
        nonZeroConstraint,
        HestonModel::FellerConstraint());

    Array params(4);
    params[0] = sTheta; params[1] = sKappa;
    params[2] = sSigma; params[3] = sX0;


    //    Optimization would take too long
    //
    //    Problem prob(costFunction, nonZeroConstraint, params);
    //
    //    Simplex simplex(0.05);
    //    simplex.minimize(prob, EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));

    const Real tol = 0.5;
    const Real costValue = costFunction.value(params);

    if (costValue > tol) {
        BOOST_FAIL("failed to reproduce small cost function value"
                << "\n    value:       " << costValue
                << "\n    tolerance:   " << tol);
    }

    const Date maturityDate = todaysDate + Period(1, Years);
    const Time maturityTime = bsProcess->time(maturityDate);

    const ext::shared_ptr<Exercise> europeanExercise(
        ext::make_shared<EuropeanExercise>(maturityDate));

    VanillaOption vanillaATMOption(
        ext::make_shared<PlainVanillaPayoff>(Option::Call,
            s0*qTS->discount(maturityDate)/rTS->discount(maturityDate)),
        europeanExercise);

    vanillaATMOption.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(hestonModel));

    const Volatility atmVol = vanillaATMOption.impliedVolatility(
        vanillaATMOption.NPV(),
        ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS,
            Handle<BlackVolTermStructure>(flatVol(std::sqrt(theta), dc))));

    const ext::shared_ptr<PricingEngine> analyticEngine(
        ext::make_shared<AnalyticDoubleBarrierBinaryEngine>(
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(flatVol(atmVol, dc)))));

    const ext::shared_ptr<PricingEngine> fdSLVEngine(
        ext::make_shared<FdHestonDoubleBarrierEngine>(
            hestonModel4slv.currentLink(),
            51, 201, 51, 1,
            FdmSchemeDesc::Hundsdorfer(), leverageFctFDM));

    const Size n = 16;
    Array barrier_lo(n), barrier_hi(n), bsNPV(n), slvNPV(n);

    const ext::shared_ptr<CashOrNothingPayoff> payoff =
        ext::make_shared<CashOrNothingPayoff>(Option::Call, 0.0, 1.0);

    for (Size i=0; i < n; ++i) {
        const Real dist = 20.0+5.0*i;

        barrier_lo[i] = std::max(s0 - dist, 1e-2);
        barrier_hi[i] = s0 + dist;
        DoubleBarrierOption doubleBarrier(
            DoubleBarrier::KnockOut, barrier_lo[i], barrier_hi[i], 0.0,
            payoff,
            europeanExercise);

        doubleBarrier.setPricingEngine(analyticEngine);
        bsNPV[i] = doubleBarrier.NPV();

        doubleBarrier.setPricingEngine(fdSLVEngine);
        slvNPV[i] = doubleBarrier.NPV();
    }


    const TimeGrid bGrid(maturityTime, tSteps);

    const PseudoRandom::ursg_type ursg = PseudoRandom::ursg_type(tSteps, 1235UL);

    std::vector<GeneralStatistics> stats(n);

    const Real df = 4*sTheta*sKappa/(sSigma*sSigma);

    for (Size i=0; i < nScenarios; ++i) {
        std::vector<bool> touch(n, false);

        const std::vector<Real>& path = ursg.nextSequence().value;

        Real x = sX0;

        for (Size j=0; j < tSteps; ++j) {
            const Time t0 = bGrid.at(j);
            const Time t1 = bGrid.at(j+1);

            const Real ncp = 4*sKappa*std::exp(-sKappa*(t1-t0))
                / (sSigma*sSigma*(1-std::exp(-sKappa*(t1-t0))))*x;

            const boost::math::non_central_chi_squared_distribution<Real>
                dist(df, ncp);

            const Real u = boost::math::quantile(dist, path[j]);

            x = sSigma*sSigma*(1-std::exp(-sKappa*(t1-t0)))/(4*sKappa) * u;

            const Real X = x*4*sKappa/(sSigma*sSigma*(1-std::exp(-sKappa*t1)));

            const Real s = gSqrt(t1, X);

            if (t1 > 0.05) {
                for (Size u=0; u < n; ++u) {
                    if (s <= barrier_lo[u] || s >= barrier_hi[u]) {
                        touch[u] = true;
                    }
                }
            }
        }
        for (Size u=0; u < n; ++u) {
            if (touch[u]) {
                stats[u].add(0.0);
            }
            else {
                stats[u].add(rTS->discount(maturityDate));
            }
        }
    }


    for (Size u=0; u < n; ++u) {
        const Real calculated = stats[u].mean();
        const Real error = stats[u].errorEstimate();
        const Real expected = slvNPV[u];

        const Real tol = 2.35*error;

        if (std::fabs(calculated-expected) > tol) {
            BOOST_FAIL("failed to reproduce CLV double no touch barrier price"
                    << "\n    CLV value:   " << calculated
                    << "\n    error    :   " << error
                    << "\n    SLV value: " << expected);
        }
    }
}

 
test_suite* SquareRootCLVModelTest::experimental() {
    auto* suite = BOOST_TEST_SUITE("SquareRootCLVModel tests");

    suite->add(QUANTLIB_TEST_CASE(
        &SquareRootCLVModelTest::testSquareRootCLVVanillaPricing));

    suite->add(QUANTLIB_TEST_CASE(
        &SquareRootCLVModelTest::testSquareRootCLVMappingFunction));

//    this test takes very long
//    suite->add(QUANTLIB_TEST_CASE(
//        &SquareRootCLVModelTest::testForwardSkew));

    return suite;
}
