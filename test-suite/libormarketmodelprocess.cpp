/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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
#include "libormarketmodelprocess.hpp"
#include "utilities.hpp"
#include <ql/timegrid.hpp>
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/MonteCarlo/multipathgenerator.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Math/generalstatistics.hpp>
#include <ql/TermStructures/zerocurve.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/Volatilities/capletconstantvol.hpp>
#include <ql/Volatilities/capletvariancecurve.hpp>
#include <ql/Processes/capletlmmprocess.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(LiborMarketModelProcessTest)

Size len = 10;

boost::shared_ptr<CapletVarianceCurve> makeCapVolCurve(const Date& refDate) {
    Volatility vols[] = {14.40, 17.15, 16.81, 16.64, 16.17,
                         15.78, 15.40, 15.21, 14.86, 14.54};

    std::vector<Date> dates;
    std::vector<Volatility> capletVols;
    for (Size i=0; i < 10; ++i) {
        capletVols.push_back(vols[i]/100);
        dates.push_back(refDate+Period(i+1, Years));
    }

    return boost::shared_ptr<CapletVarianceCurve>(
                         new CapletVarianceCurve(refDate, dates,
                                                 capletVols, ActualActual()));
}

boost::shared_ptr<Xibor> makeIndex() {
    DayCounter dayCounter = Actual360();
    std::vector<Date> dates;
    std::vector<Rate> rates;
    dates.push_back(Date(4,September,2005));
    dates.push_back(Date(4,September,2018));
    rates.push_back(0.01);
    rates.push_back(0.08);

    Handle<YieldTermStructure> termStructure(
                      boost::shared_ptr<YieldTermStructure>(
                                      new ZeroCurve(dates,rates,dayCounter)));

    boost::shared_ptr<Xibor> index(new Euribor(1, Years, termStructure));

    Date todaysDate = index->calendar().adjust(Date(4,September,2005));
    Settings::instance().evaluationDate() = todaysDate;

    dates[0] = index->calendar().advance(todaysDate,
                                         index->settlementDays(), Days);

    termStructure.linkTo(boost::shared_ptr<YieldTermStructure>(
                                    new ZeroCurve(dates, rates, dayCounter)));

    return index;
}

boost::shared_ptr<CapletLiborMarketModelProcess>
makeProcess(const Matrix& volaComp = Matrix()) {
    boost::shared_ptr<Xibor> index = makeIndex();
    return boost::shared_ptr<CapletLiborMarketModelProcess>(
        new CapletLiborMarketModelProcess(
                     len, index,
                     makeCapVolCurve(index->termStructure()->referenceDate()),
                     volaComp));
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(LiborMarketModelProcessTest)


void LiborMarketModelProcessTest::testInitialisation() {
    BOOST_MESSAGE("Testing caplet LMM process initialisation...");

    QL_TEST_BEGIN

    DayCounter dayCounter = Actual360();
    Handle<YieldTermStructure> termStructure(
        flatRate(Date::todaysDate(), 0.04, dayCounter));

    boost::shared_ptr<Xibor> index(new Euribor(6, Months, termStructure));
    boost::shared_ptr<CapletVolatilityStructure> capletVol(
        new CapletConstantVolatility(termStructure->referenceDate(), 0.2,
                                     termStructure->dayCounter()));

    Calendar calendar = index->calendar();

    for (Integer daysOffset=0; daysOffset < 1825 /* 5 year*/; daysOffset+=8) {
        Date todaysDate = calendar.adjust(Date::todaysDate()+daysOffset);
        Settings::instance().evaluationDate() = todaysDate;
        Date settlementDate =
            calendar.advance(todaysDate, index->settlementDays(), Days);

        termStructure.linkTo(flatRate(settlementDate, 0.04, dayCounter));

        CapletLiborMarketModelProcess process(60, index, capletVol);
    }

    QL_TEST_TEARDOWN
}

void LiborMarketModelProcessTest::testLambdaBootstrapping() {
    BOOST_MESSAGE("Testing caplet-LMM lambda bootstrapping...");

    QL_TEST_BEGIN

    Real tolerance = 1e-8;
    Volatility lambdaExpected[] = {14.30102976,19.37000802,16.00220245,
                                   15.99965627,14.05756850,13.57282207,
                                   12.79058927,13.69861487,11.61842263};

    boost::shared_ptr<CapletLiborMarketModelProcess> process = makeProcess();

    for (Size i=0; i<9; ++i) {
        if (std::fabs(process->lambda(i) - lambdaExpected[i]/100) > tolerance)
            BOOST_ERROR("Failed to reproduce expected lambda values"
                        << "\n    calculated: " << process->lambda(i)
                        << "\n    expected:   " << lambdaExpected[i]/100);
    }

    QL_TEST_TEARDOWN
}

void LiborMarketModelProcessTest::testMonteCarloCapletPricing() {
    BOOST_MESSAGE("Testing caplet-LMM Monte-Carlo caplet pricing...");

    QL_TEST_BEGIN

    /* factor loadings are taken from Hull & White article
       http://www.rotman.utoronto.ca/~amackay/fin/libormktmodel2.pdf */
    Real compValues[] = {0.852, 0.467, 0.236,
                         0.917, 0.377, 0.127,
                         0.964, 0.264, 0.000,
                         0.982, 0.135,-0.136,
                         0.964, 0.000,-0.267,
                         0.982,-0.135,-0.136,
                         0.964,-0.264, 0.000,
                         0.917,-0.377, 0.127,
                         0.852,-0.467, 0.236};

    Matrix volaComp(9,3);
    std::copy(compValues, compValues+9*3, volaComp.begin());

    boost::shared_ptr<CapletLiborMarketModelProcess> process1 = makeProcess();
    boost::shared_ptr<CapletLiborMarketModelProcess> process2 = makeProcess(
                                                                    volaComp);

    std::vector<Time> tmp = process1->fixingTimes();
    TimeGrid grid(tmp.begin(), tmp.end());

    Size i;
    std::vector<Size> location;
    for (i=0; i < tmp.size(); ++i) {
        location.push_back(
                      std::find(grid.begin(),grid.end(),tmp[i])-grid.begin());
    }

    // set-up a small Monte-Carlo simulation to price caplets
    // and ratchet caps using a one- and a three factor libor market model
    typedef LowDiscrepancy::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    BigNatural seed = 42;
    rsg_type rsg1 = LowDiscrepancy::make_sequence_generator(
                            process1->factors()*(grid.size()-1), seed);
    rsg_type rsg2 = LowDiscrepancy::make_sequence_generator(
                            process2->factors()*(grid.size()-1), seed);
    MultiPathGenerator<rsg_type> generator1(process1, grid, rsg1, false);
    MultiPathGenerator<rsg_type> generator2(process2, grid, rsg2, false);

    const Size nrTrails = 100000;
    std::vector<GeneralStatistics> stat1(process1->size());
    std::vector<GeneralStatistics> stat2(process2->size());
    std::vector<GeneralStatistics> stat3(process2->size()-1);
    for (i=0; i<nrTrails; ++i) {
        sample_type path1 = generator1.next();
        sample_type path2 = generator2.next();

        std::vector<Rate> rates1(len);
        std::vector<Rate> rates2(len);
        for (Size j=0; j<process1->size(); ++j) {
            rates1[j] = path1.value[j][location[j]];
            rates2[j] = path2.value[j][location[j]];
        }

        for (Size k=0; k<process1->size(); ++k) {
            // caplet payoff function, cap rate at 4%
            Real payoff1 = std::max(rates1[k] - 0.04, 0.0)
                          * process1->accrualPeriod(k);

            Real payoff2 = std::max(rates2[k] - 0.04, 0.0)
                          * process2->accrualPeriod(k);
            stat1[k].add(process1->discountBond(rates1, k) * payoff1);
            stat2[k].add(process2->discountBond(rates2, k) * payoff2);

            if (k != 0) {
                // ratchet cap payoff function
                Real payoff3 =  std::max(rates2[k] - (rates2[k-1]+0.0025), 0.0)
                              * process2->accrualPeriod(k);
                stat3[k-1].add(process2->discountBond(rates2, k) * payoff3);
            }
        }
    }

    Real capletNpv[] = {0.00000000,0.00000284,0.00253328,
                        0.00957727,0.01774680,0.02521635,
                        0.03160823,0.03664568,0.03979225,
                        0.04182989};

    Real ratchetNpv[] = {0.008265,0.008275,0.008215,
                         0.008297,0.008380,0.008435,
                         0.008412,0.008178,0.007952};

    for (Size k=0; k < process1->size(); ++k) {

        Real calculated1 = stat1[k].mean();
        Real tolerance1  = stat1[k].errorEstimate();
        Real expected    = capletNpv[k];

        if (std::fabs(calculated1 - expected) > tolerance1) {
            BOOST_ERROR("Failed to reproduce expected caplet NPV"
                        << "\n    calculated: " << calculated1
                        << "\n    error int:  " << tolerance1
                        << "\n    expected:   " << expected);
        }

        Real calculated2 = stat2[k].mean();
        Real tolerance2  = stat2[k].errorEstimate();

        if (std::fabs(calculated2 - expected) > tolerance2) {
            BOOST_ERROR("Failed to reproduce expected caplet NPV"
                        << "\n    calculated: " << calculated2
                        << "\n    error int:  " << tolerance2
                        << "\n    expected:   " << expected);
        }

        if (k != 0) {
            Real calculated3 = stat3[k-1].mean();
            Real tolerance3  = stat3[k-1].errorEstimate();
            expected    = ratchetNpv[k-1];

            Real refError = 1e-5; // 1e-5. error bars of the reference values

            if (std::fabs(calculated3 - expected) > tolerance3 + refError) {
                BOOST_ERROR("Failed to reproduce expected caplet NPV"
                            << "\n    calculated: " << calculated3
                            << "\n    error int:  " << tolerance3 + refError
                            << "\n    expected:   " << expected);
            }
        }
    }

    QL_TEST_TEARDOWN
}

test_suite* LiborMarketModelProcessTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Libor market model process tests");

    suite->add(BOOST_TEST_CASE(
        &LiborMarketModelProcessTest::testInitialisation));
    suite->add(BOOST_TEST_CASE(
        &LiborMarketModelProcessTest::testLambdaBootstrapping));
    suite->add(BOOST_TEST_CASE(
        &LiborMarketModelProcessTest::testMonteCarloCapletPricing));
    return suite;
}

