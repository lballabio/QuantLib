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

#include <ql/quotes/simplequote.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <experimental/finitedifferences/bsmrndcalculator.hpp>
#include <experimental/finitedifferences/hestonrndcalculator.hpp>

#include <boost/make_shared.hpp>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void RiskNeutralDensityCalculatorTest::testDensityAgainstOptionPrices() {
    BOOST_TEST_MESSAGE("Testing density against option prices ...");

    SavedSettings backup;

    const DayCounter dayCounter = Actual365Fixed();
    const Date settlementDate = Settings::instance().evaluationDate();

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.075;
    const Rate q = 0.04;
    const Volatility v = 0.27;

	const boost::shared_ptr<YieldTermStructure> rTS(
    	flatRate(settlementDate, r, dayCounter));

    const boost::shared_ptr<YieldTermStructure> qTS(
    	flatRate(settlementDate, q, dayCounter));

    const BSMRNDCalculator bsm(x0, v, rTS, qTS);
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
    const Date settlementDate = Settings::instance().evaluationDate();

    const Real s0 = 10;
    const Real x0 = std::log(s0);
    const Rate r = 0.155;
    const Rate q = 0.0721;
    const Volatility v = 0.27;

    const Real kappa = 1.0;
    const Real theta = v*v;
    const Real rho = -0.75;
    const Real v0 = v*v;
    const Real sigma = 0.0001;

	const boost::shared_ptr<YieldTermStructure> rTS(
    	flatRate(settlementDate, r, dayCounter));

    const boost::shared_ptr<YieldTermStructure> qTS(
    	flatRate(settlementDate, q, dayCounter));

    const BSMRNDCalculator bsm(x0, v, rTS, qTS);
    const HestonRNDCalculator heston(
    	boost::make_shared<HestonProcess>(
    		Handle<YieldTermStructure>(rTS),
			Handle<YieldTermStructure>(qTS),
    	    Handle<Quote>(boost::make_shared<SimpleQuote>(s0)),
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

test_suite* RiskNeutralDensityCalculatorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Risk neutral density calculator tests");
    suite->add(QUANTLIB_TEST_CASE(
    	&RiskNeutralDensityCalculatorTest::testDensityAgainstOptionPrices));

    suite->add(QUANTLIB_TEST_CASE(
        	&RiskNeutralDensityCalculatorTest::testBSMagainstHestonRND));
    return suite;
}
