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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/bonds/amortizingfixedratebond.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, QuantLib::TopLevelFixture)

BOOST_AUTO_TEST_SUITE(AmortizingBondTest)

BOOST_AUTO_TEST_CASE(testAmortizingFixedRateBond) {
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

    Date refDate = Settings::instance().evaluationDate();

    const Real tolerance = 1.0e-6;

    for (Size i=0; i<LENGTH(rates); ++i) {

        auto schedule = sinkingSchedule(refDate, Period(30, Years), freq, NullCalendar());
        auto notionals = sinkingNotionals(Period(30, Years), freq, rates[i], 100.0);

        AmortizingFixedRateBond myBond(0, notionals, schedule, {rates[i]},
                                       ActualActual(ActualActual::ISMA));

        Leg cashflows = myBond.cashflows();

        for (Size k=0; k < cashflows.size() / 2; ++k) {
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
            Real expectedCoupon = notionals[k] * rates[i] / Integer(freq);
            error = std::fabs(coupon - expectedCoupon);

            if (error > tolerance) {
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

BOOST_AUTO_TEST_CASE(testBrazilianAmortizingFixedRateBond) {
    BOOST_TEST_MESSAGE("Testing Brazilian amortizing fixed rate bond...");

    /*
    * Following data is based on the following Brazilian onshore corporate bond code:
    * SND Code - RISF11
    * ISIN Code - BRRISFDBS005
    * Fiduciary Agent URL - https://www.pentagonotrustee.com.br/Site/DetalhesEmissor?ativo=RISF11&aba=tab-5&tipo=undefined
    */

    static const Real arr[] = {
        1000		, 983.33300000, 966.66648898, 950.00019204,
        933.33338867, 916.66685434, 900.00001759, 883.33291726,
        866.66619177, 849.99933423, 833.33254728, 816.66589633,
        799.99937871, 783.33299165, 766.66601558, 749.99946306,
        733.33297499, 716.66651646, 699.99971995, 683.33272661,
        666.66624140, 649.99958536, 633.33294599, 616.66615618,
        599.99951997, 583.33273330, 566.66633377, 549.99954356,
        533.33290739, 516.66625403, 499.99963400, 483.33314619,
        466.66636930, 449.99984658, 433.33320226, 416.66634063,
        399.99968700, 383.33290004, 366.66635221, 349.99953317,
        333.33290539, 316.66626012, 299.99948151, 283.33271031,
        266.66594695, 249.99932526, 233.33262024, 216.66590450,
        199.99931312, 183.33277035, 166.66617153, 149.99955437,
        133.33295388, 116.66633464,  99.99973207,  83.33307672,
         66.66646137,  49.99984602,  33.33324734,  16.66662367
    };
    std::vector<Real> notionals (arr, arr + sizeof(arr) / sizeof(arr[0]) );

    Real expected_amortizations[] = {
        16.66700000, 16.66651102, 16.66629694, 16.66680337,
        16.66653432, 16.66683675, 16.66710033, 16.66672548,
        16.66685753, 16.66678695, 16.66665095, 16.66651761,
        16.66638706, 16.66697606, 16.66655251, 16.66648807,
        16.66645852, 16.66679651, 16.66699333, 16.66648520,
        16.66665604, 16.66663937, 16.66678981, 16.66663620,
        16.66678667, 16.66639952, 16.66679021, 16.66663617,
        16.66665336, 16.66662002, 16.66648780, 16.66677688,
        16.66652271, 16.66664432, 16.66686163, 16.66665363,
        16.66678696, 16.66654783, 16.66681904, 16.66662777,
        16.66664527, 16.66677860, 16.66677119, 16.66676335,
        16.66662168, 16.66670502, 16.66671573, 16.66659137,
        16.66654276, 16.66659882, 16.66661715, 16.66660049,
        16.66661924, 16.66660257, 16.66665534, 16.66661534,
        16.66661534, 16.66659867, 16.66662367, 16.66662367
    };

    Real expected_coupons[] = {
        5.97950399, 4.85474255, 5.27619136, 5.18522454,
        5.33753111, 5.24221882, 4.91231709, 4.59116258,
        4.73037674, 4.63940686, 4.54843737, 3.81920094,
        4.78359948, 3.86733691, 4.38439657, 4.09359456,
        4.00262671, 4.28531030, 3.82068947, 3.55165259,
        3.46502778, 3.71720657, 3.62189368, 2.88388676,
        3.58769952, 2.72800044, 3.38838360, 3.00196900,
        2.91100034, 3.08940793, 2.59877059, 2.63809514,
        2.42551945, 2.45615766, 2.59111761, 1.94857222,
        2.28751141, 1.79268582, 2.19248291, 1.81913832,
        1.90625855, 1.89350716, 1.48110584, 1.62031828,
        1.38600825, 1.23425366, 1.39521333, 1.06968563,
        1.03950542, 1.00065409, 0.90968563, 0.81871706,
        0.79726493, 0.63678002, 0.57187676, 0.49829046,
        0.32913418, 0.27290565, 0.19062560, 0.08662552
    };

    Natural settlementDays = 0;
    Date issueDate(2, March, 2020);
    Date maturityDate(2, March, 2025);

    Schedule schedule(issueDate,
                        maturityDate,
                        Period(Monthly),
                        Brazil(Brazil::Settlement),
                        Unadjusted,
                        Unadjusted,
                        DateGeneration::Backward,
                        false);

    std::vector<InterestRate> couponRates = {
        InterestRate(0.0675,
                     Business252(Brazil()),
                     Compounded, Annual)
    };

    Leg coupons = FixedRateLeg(schedule)
        .withNotionals(notionals)
        .withCouponRates(couponRates)
        .withPaymentAdjustment(Following);

    Bond risf11(settlementDays,
                schedule.calendar(),
                issueDate,
                coupons);

    const Real tolerance = 1.0e-6;
    Real error;
    Leg cashflows = risf11.cashflows();
    for (Size k=0; k < cashflows.size() / 2; ++k) {
        error = std::fabs(expected_coupons[k] - cashflows[2*k]->amount());
        if(error > tolerance) {
            BOOST_ERROR("\n" <<
                " " << k << "th cash flow "
                " Failed!" <<
                " Expected Coupon: " << expected_coupons[k] <<
                " Calculated Coupon: " << cashflows[2*k]->amount());
        }

        error = std::fabs(expected_amortizations[k]- cashflows[2*k+1]->amount());
        if(error > tolerance) {
            BOOST_ERROR("\n" <<
                " " << k << "th cash flow "
                " Failed!" <<
                " Expected Amortization: " << expected_amortizations[k] <<
                " Calculated Amortization: " << cashflows[2*k+1]->amount());
        }

    }

}

BOOST_AUTO_TEST_CASE(testAmortizingFixedRateBondWithDrawDown) {
    BOOST_TEST_MESSAGE("Testing amortizing fixed rate bond with draw-down...");

    Date issueDate = Date(19, May, 2012);
    Date maturityDate = Date(25, May, 2017);
    Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
    Natural settlementDays = 3;

    Schedule schedule(issueDate, maturityDate, Period(Semiannual), calendar,
                      Unadjusted, Unadjusted, DateGeneration::Backward, false);

    std::vector<Real> nominals = { 100.0, 100.0, 100.5, 100.5, 101.5, 101.5, 90.0, 80.0, 70.0, 60.0 };
    std::vector<Real> rates = { 0.042 };

    Leg leg = FixedRateLeg(schedule)
        .withNotionals(nominals)
        .withCouponRates(rates, Actual360())
        .withPaymentAdjustment(Unadjusted)
        .withPaymentCalendar(calendar);

    Bond bond(settlementDays, calendar, issueDate, leg);

    const auto& cfs = bond.cashflows();

    // first draw-down
    Real calculated = cfs.at(2)->amount();
    Real expected = nominals[1] - nominals[2];
    Real error = std::fabs(calculated - expected);
    Real tolerance = 1e-8;

    if(error > tolerance) {
        BOOST_ERROR("Failed to calculate first draw down: "
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated);
    }

    // second draw-down
    calculated = cfs.at(5)->amount();
    expected = nominals[3] - nominals[4];
    error = std::fabs(calculated - expected);

    if(error > tolerance) {
        BOOST_ERROR("Failed to calculate second draw down: "
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated);
    }

    // first amortization
    calculated = cfs.at(8)->amount();
    expected = nominals[5] - nominals[6];
    error = std::fabs(calculated - expected);

    if(error > tolerance) {
        BOOST_ERROR("Failed to calculate fist amortization: "
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated);
    }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
