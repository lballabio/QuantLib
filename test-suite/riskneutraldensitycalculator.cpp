/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

#include "riskneutraldensitycalculator.hpp"
#include "utilities.hpp"

#include <ql/timegrid.hpp>
#include <ql/math/functional.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <experimental/finitedifferences/bsmrndcalculator.hpp>
#include <experimental/finitedifferences/hestonrndcalculator.hpp>
#include <experimental/finitedifferences/localvolrndcalculator.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void RiskNeutralDensityCalculatorTest::testDensityAgainstOptionPrices() {
    BOOST_TEST_MESSAGE("Testing density against option prices ...");

    SavedSettings backup;

    const DayCounter dayCounter = Actual365Fixed();
    const Date todaysDate = Settings::instance().evaluationDate();

    const Real s0 = 100;
    const Handle<Quote> spot(
    	boost::shared_ptr<SimpleQuote>(new SimpleQuote(s0)));

    const Rate r = 0.075;
    const Rate q = 0.04;
    const Volatility v = 0.27;

	const Handle<YieldTermStructure> rTS(flatRate(todaysDate, r, dayCounter));

    const Handle<YieldTermStructure> qTS(flatRate(todaysDate, q, dayCounter));

    const boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
    	new BlackScholesMertonProcess(
    		spot, qTS, rTS,
			Handle<BlackVolTermStructure>(flatVol(v, dayCounter))));

    const BSMRNDCalculator bsm(bsmProcess);
    const Time times[] = { 0.5, 1.0, 2.0 };
    const Real strikes[] = { 75.0, 100.0, 150.0 };

    for (Size i=0; i < LENGTH(times); ++i) {
    	const Time t = times[i];
    	const Volatility stdDev = v*std::sqrt(t);
    	const DiscountFactor df = rTS->discount(t);
    	const Real fwd = s0*qTS->discount(t)/df;

    	for (Size j=0; j < LENGTH(strikes); ++j) {
    		const Real strike = strikes[j];
    		const Real xs = std::log(strike);
            const BlackCalculator blackCalc(
            	Option::Put, strike, fwd, stdDev, df);

            const Real tol = std::sqrt(QL_EPSILON);
    		const Real calculatedCDF = bsm.cdf(xs, t);
    		const Real expectedCDF
				= blackCalc.strikeSensitivity()/df;

    		if (std::fabs(calculatedCDF - expectedCDF) > tol) {
    			BOOST_FAIL("failed to reproduce Black-Scholes-Merton cdf"
    					<< "\n   calculated: " << calculatedCDF
						<< "\n   expected:   " << expectedCDF
						<< "\n   diff:       " << calculatedCDF - expectedCDF
						<< "\n   tol:        " << tol);
    		}

    		const Real deltaStrike = strike*std::sqrt(QL_EPSILON);

    		const Real calculatedPDF = bsm.pdf(xs, t);
    		const Real expectedPDF = strike/df*
				(  BlackCalculator(Option::Put, strike+deltaStrike,
				       fwd, stdDev, df).strikeSensitivity()
				 - BlackCalculator(Option::Put, strike - deltaStrike,
				  	   fwd, stdDev, df).strikeSensitivity())/(2*deltaStrike);

    		if (std::fabs(calculatedPDF - expectedPDF) > tol) {
    			BOOST_FAIL("failed to reproduce Black-Scholes-Merton pdf"
    					<< "\n   calculated: " << calculatedPDF
						<< "\n   expected:   " << expectedPDF
						<< "\n   diff:       " << calculatedPDF - expectedPDF
						<< "\n   tol:        " << tol);
    		}
    	}
    }
}

void RiskNeutralDensityCalculatorTest::testBSMagainstHestonRND() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes-Merton and Hston densities ...");

    SavedSettings backup;

    const DayCounter dayCounter = Actual365Fixed();
    const Date todaysDate = Settings::instance().evaluationDate();

    const Real s0 = 10;
    const Handle<Quote> spot(
    	boost::shared_ptr<SimpleQuote>(new SimpleQuote(s0)));

    const Rate r = 0.155;
    const Rate q = 0.0721;
    const Volatility v = 0.27;

    const Real kappa = 1.0;
    const Real theta = v*v;
    const Real rho = -0.75;
    const Real v0 = v*v;
    const Real sigma = 0.0001;

	const Handle<YieldTermStructure> rTS(flatRate(todaysDate, r, dayCounter));

    const Handle<YieldTermStructure> qTS(flatRate(todaysDate, q, dayCounter));

    const boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
    	new BlackScholesMertonProcess(
    		spot, qTS, rTS,
			Handle<BlackVolTermStructure>(flatVol(v, dayCounter))));

    const BSMRNDCalculator bsm(bsmProcess);
    const HestonRNDCalculator heston(
    	boost::make_shared<HestonProcess>(
    		rTS, qTS, spot,
    		v0, kappa, theta, sigma, rho), 1e-8);

    const Time times[] = { 0.5, 1.0, 2.0 };
    const Real strikes[] = { 7.5, 10, 15 };
    const Real probs[] = { 1e-6, 0.01, 0.5, 0.99, 1.0-1e-6 };

    for (Size i=0; i < LENGTH(times); ++i) {
    	const Time t = times[i];

    	for (Size j=0; j < LENGTH(strikes); ++j) {
    		const Real strike = strikes[j];
    		const Real xs = std::log(strike);

    		const Real expectedPDF = bsm.pdf(xs, t);
    		const Real calculatedPDF = heston.pdf(xs, t);

    		const Real tol = 1e-4;
    		if (std::fabs(expectedPDF - calculatedPDF) > tol) {
    			BOOST_FAIL("failed to reproduce Black-Scholes-Merton pdf "
    					   "with the Heston model"
    					<< "\n   calculated: " << calculatedPDF
						<< "\n   expected:   " << expectedPDF
						<< "\n   diff:       " << calculatedPDF - expectedPDF
						<< "\n   tol:        " << tol);
    		}

    		const Real expectedCDF = bsm.cdf(xs, t);
    		const Real calculatedCDF = heston.cdf(xs, t);

    		if (std::fabs(expectedCDF - calculatedCDF) > tol) {
    			BOOST_FAIL("failed to reproduce Black-Scholes-Merton cdf "
    					   "with the Heston model"
    					<< "\n   calculated: " << calculatedCDF
						<< "\n   expected:   " << expectedCDF
						<< "\n   diff:       " << calculatedCDF - expectedCDF
						<< "\n   tol:        " << tol);
    		}
    	}

		for (Size j=0; j < LENGTH(probs); ++j) {
			const Real prob = probs[j];
    		const Real expectedInvCDF = bsm.invcdf(prob, t);
    		const Real calculatedInvCDF = heston.invcdf(prob, t);

    		const Real tol = 1e-3;
    		if (std::fabs(expectedInvCDF - calculatedInvCDF) > tol) {
    			BOOST_FAIL("failed to reproduce Black-Scholes-Merton "
    					"inverse cdf with the Heston model"
    					<< "\n   calculated: " << calculatedInvCDF
						<< "\n   expected:   " << expectedInvCDF
						<< "\n   diff:       " << calculatedInvCDF - expectedInvCDF
						<< "\n   tol:        " << tol);
    		}
    	}
    }
}

namespace {
	// see Peter Jaeckel, Hyperbolic local volatility
	// http://www.jaeckel.org/HyperbolicLocalVolatility.pdf
	class HyperbolicLocalVolatility : public LocalVolTermStructure {
	  public:
		HyperbolicLocalVolatility(Real s0, Real beta, Volatility sig)
	  	: LocalVolTermStructure(Following, Actual365Fixed()),
		  s0_(s0), b_(beta), sig_(sig) {}

		Date maxDate() const { return Date::maxDate(); }
		Rate minStrike() const { return 0.0; }
		Rate maxStrike() const { return QL_MAX_REAL; }

	  private:
		Volatility localVolImpl(Time, Real s) const {
			const Real x = s/s0_;
			const Real b2 = b_*b_;

			const Real h = (1 - b_+b2)/b_ * x
				+ (b_-1)/b_*(std::sqrt(x*x + b2*square<Real>()(1-x)) - b_);

			return sig_*h;
		}

		const Real s0_, b_;
		const Volatility sig_;
	};
}

void RiskNeutralDensityCalculatorTest::testLocalVolatilityRND() {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
    				   "for local volatility process to calculate "
    				   "risk neutral densities ...");

    SavedSettings backup;

    const DayCounter dayCounter = Actual365Fixed();
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Rate r    = 0.05;
    const Rate q    = 0.025;
    const Real s0   = 100;
    const Real beta = 0.25;
    const Real sig  = 0.25;

	const boost::shared_ptr<Quote> spot(
		boost::make_shared<SimpleQuote>(s0));
	const boost::shared_ptr<YieldTermStructure> rTS(
		flatRate(todaysDate, r, dayCounter));
	const boost::shared_ptr<YieldTermStructure> qTS(
		flatRate(todaysDate, q, dayCounter));

	const boost::shared_ptr<LocalVolTermStructure> localVol(
		new HyperbolicLocalVolatility(s0, beta, sig));

	const boost::shared_ptr<TimeGrid> timeGrid(new TimeGrid(1.0, 26));

	const boost::shared_ptr<LocalVolRNDCalculator> rndCalc(
		new LocalVolRNDCalculator(spot, rTS, qTS, localVol, timeGrid));

	rndCalc->pdf(1.0, 1.0);
//	std::cout << rndCalc->x(timeGrid->at(26)+1) << std::endl;
}



test_suite* RiskNeutralDensityCalculatorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Risk neutral density calculator tests");

    suite->add(QUANTLIB_TEST_CASE(
    	&RiskNeutralDensityCalculatorTest::testDensityAgainstOptionPrices));
    suite->add(QUANTLIB_TEST_CASE(
        &RiskNeutralDensityCalculatorTest::testBSMagainstHestonRND));
    suite->add(QUANTLIB_TEST_CASE(
    	&RiskNeutralDensityCalculatorTest::testLocalVolatilityRND));
    return suite;
}
