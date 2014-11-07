/*
 Copyright (C) 2014 Cheng Li

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

#include "amortizingbond.hpp"
#include "utilities.hpp"
#include <ql/experimental/amortizingbonds/amortizingfixedratebond.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void AmortizingBondTest::testAmortizingFixedRateBond() {
	BOOST_TEST_MESSAGE("Testing amortizing fixed rate bond...");

	/*
	* Following data is generated from Excel using function pmt with Nper = 360, PV = 100.0 
	*/

	Real rates[] = {0.0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12};
	Real amounts[] = {0.277777778, 0.321639520, 0.369619473, 0.421604034,
		              0.477415295, 0.536821623, 0.599550525, 
		              0.665302495, 0.733764574, 0.804622617,
		              0.877571570, 0.952323396, 1.028612597};

	Frequency freq = Monthly;

	Date refDate = Date::todaysDate();

	const Real tolerance = 1.0e-6;

	for(Size i=0; i<LENGTH(rates); ++i) {

		AmortizingFixedRateBond
			myBond(0, NullCalendar(), 100.0, refDate, Period(30, Years), freq, rates[i], ActualActual(ActualActual::ISMA));

		Leg cashflows = myBond.cashflows();

		std::vector<Real> notionals = myBond.notionals();

		for(Size k=0; k < cashflows.size() / 2; ++k) {
			Real coupon = cashflows[2*k]->amount();
			Real principal = cashflows[2*k+1]->amount();
			Real totalAmount = coupon + principal;

			// Check the amount is same as pmt returned

			Real error = std::fabs(totalAmount-amounts[i]);
			if (error > tolerance) {
				BOOST_ERROR("\n" <<
					        " Rate: " << rates[i] <<
							" " << k << "th cash flow " 
							" Failed!" <<
							" Expected Amount: " << amounts[i] <<
							" Calculated Amount: " << totalAmount);
			}

			// Check the coupon result
			Real expectedCoupon = notionals[k] * rates[i] / freq;
			error = std::fabs(coupon- expectedCoupon);

			if(error > tolerance) {
				BOOST_ERROR("\n" <<
					" Rate: " << rates[i] <<
					" " << k << "th cash flow " 
					" Failed!" <<
					" Expected Coupon: " << expectedCoupon <<
					" Calculated Coupon: " << coupon);
			}

		}
	}
}

test_suite* AmortizingBondTest::suite() {
	test_suite* suite = BOOST_TEST_SUITE("Amortizing Bond tests");
	suite->add(
		QUANTLIB_TEST_CASE(&AmortizingBondTest::testAmortizingFixedRateBond));
	return suite;
}
