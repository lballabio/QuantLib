/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include "montecarlo_multithreaded.hpp"
#include "utilities.hpp"
#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/pricingengines/mclongstaffschwartzengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/mcamericanengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/vanilla/mceuropeanhestonengine.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/period.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/math/rngtraits_multithreaded.hpp>

#include <boost/make_shared.hpp>

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

void MonteCarloMultiThreadedTest::testHestonEngine() {

#if !defined(_OPENMP)

    BOOST_TEST_MESSAGE("Skipping multithreaded Monte Carlo Heston engine test, "
                       "because OpenMP is not enabled");

#else

    // this is taken from the Heston model tests
    BOOST_TEST_MESSAGE("Testing multithreaded Monte Carlo Heston engine "
                       "against cached values...");

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(1.05)));

    boost::shared_ptr<HestonProcess> process(
        new HestonProcess(riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8,
                          HestonProcess::QuadraticExponentialMartingale));

    VanillaOption option(payoff, exercise);

    boost::shared_ptr<PricingEngine> engine;

    // use a multithreaded RNG and more samples than in the original test
    // to produce some traffic

    // PseudoRandomMultiThreaded can only handle 8 threads
    if (omp_get_max_threads() > 8)
        omp_set_num_threads(8);

    engine = MakeMCEuropeanHestonEngine<PseudoRandomMultiThreaded>(process)
                 .withStepsPerYear(11)
                 .withAntitheticVariate()
                 .withSamples(500000)
                 .withSeed(1234);

    option.setPricingEngine(engine);

    Real expected = 0.0632851308977151;
    Real calculated = option.NPV();
    Real errorEstimate = option.errorEstimate();
    Real tolerance = 7.5e-4;

    if (std::fabs(calculated - expected) > 2.34 * errorEstimate) {
        BOOST_ERROR("Failed to reproduce cached price"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected << " +/- "
                    << errorEstimate);
    }

    if (errorEstimate > tolerance) {
        BOOST_ERROR("failed to reproduce error estimate"
                    << "\n    calculated: " << errorEstimate
                    << "\n    expected:   " << tolerance);
    }

#endif
}

void MonteCarloMultiThreadedTest::testAmericanOption() {
#if !defined(_OPENMP)

    BOOST_TEST_MESSAGE(
        "Skipping multithreaded Monte Carlo LongstaffSchwartz engine test, "
        "because OpenMP is not enabled");

#else

    // this is taken from the Longstaff Schwartz engine tests
    BOOST_TEST_MESSAGE(
        "Testing multithreaded Monte Carlo LongstaffSchwartz engine "
        "against fd engine ...");

#endif

    SavedSettings backup;

    // most of the example taken from the EquityOption.cpp
    const Option::Type type(Option::Put);
    const Real underlying = 36;
    const Spread dividendYield = 0.00;
    const Rate riskFreeRate = 0.06;
    const Volatility volatility = 0.20;

    const Date todaysDate(15, May, 1998);
    const Date settlementDate(17, May, 1998);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturity(17, May, 1999);
    const DayCounter dayCounter = Actual365Fixed();

    boost::shared_ptr<Exercise> americanExercise(
        new AmericanExercise(settlementDate, maturity));

    // bootstrap the yield/dividend/vol curves
    Handle<YieldTermStructure> flatTermStructure(
        boost::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, riskFreeRate, dayCounter)));
    Handle<YieldTermStructure> flatDividendTS(
        boost::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, dividendYield, dayCounter)));

    LsmBasisSystem::PolynomType polynomTypes[] = {
        LsmBasisSystem::Monomial, LsmBasisSystem::Laguerre,
        LsmBasisSystem::Hermite, LsmBasisSystem::Hyperbolic,
        LsmBasisSystem::Chebyshev2nd};

    // expected values, precalculated with
    // FDAmericanEngine<CrankNicolson>(stochasticProcess, 401, 200);
    Real expected[2][3] = {{2.0868301995, 3.43021667169, 4.78542893195},
                           {4.48541742599, 5.73674283486, 7.1076949469}};

    // PseudoRandomMultiThreaded can only handle 8 threads
    if (omp_get_max_threads() > 8)
        omp_set_num_threads(8);

    for (Integer i = 0; i < 2; ++i) {
        for (Integer j = 0; j < 3; ++j) {
            Handle<BlackVolTermStructure> flatVolTS(
                boost::shared_ptr<BlackVolTermStructure>(
                    new BlackConstantVol(settlementDate, NullCalendar(),
                                         volatility + 0.1 * j, dayCounter)));

            boost::shared_ptr<StrikedTypePayoff> payoff(
                new PlainVanillaPayoff(type, underlying + 4 * i));

            Handle<Quote> underlyingH(
                boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

            boost::shared_ptr<GeneralizedBlackScholesProcess> stochasticProcess(
                new GeneralizedBlackScholesProcess(
                    underlyingH, flatDividendTS, flatTermStructure, flatVolTS));

            VanillaOption americanOption(payoff, americanExercise);

            boost::shared_ptr<PricingEngine> mcengine =
                MakeMCAmericanEngine<PseudoRandomMultiThreaded>(
                    stochasticProcess)
                    .withSteps(75)
                    .withAntitheticVariate()
                    .withAbsoluteTolerance(0.02)
                    .withSeed(42)
                    .withPolynomOrder(3)
                    .withBasisSystem(
                        polynomTypes[0 * (i * 3 + j) % LENGTH(polynomTypes)]);

            americanOption.setPricingEngine(mcengine);
            // FLOATING_POINT_EXCEPTION
            const Real calculated = americanOption.NPV();
            const Real errorEstimate = americanOption.errorEstimate();

            if (std::fabs(calculated - expected[i][j]) > 2.34 * errorEstimate) {
                BOOST_ERROR("Failed to reproduce american option prices"
                            << "\n    expected: " << expected
                            << "\n    calculated:   " << calculated << " +/- "
                            << errorEstimate);
            }
        }
    }
}

void MonteCarloMultiThreadedTest::testDynamicCreatorWrapper() {

    BOOST_TEST_MESSAGE("Testing dynamic creator wrapper ...");

    // Reference results obtained from the original dcmt library 0.6.2
    // A mersenne twister instance is dynamically created using
    // a creator seed 42, w = 32, p = 521 and id = 4138 and the first
    // 500 random numbers are generated.
    // The essential code for this is
    // mt_struct *mts = get_mt_parameter_id_st(32, 521, 4138, 42);
    // sgenrand_mt(1234, mts);
    // x = genrand_mt(mts);

    unsigned int dcmt_sample[] = {
        2679787694, 3087444478, 3977485442, 2542634463, 2791530108, 4133369918,
        1535500089, 420671898,  421264076,  361846315,  284466921,  3574595117,
        4153426529, 1930472333, 2376958975, 3881999471, 2050659355, 3464582270,
        100805932,  3126606429, 4290402713, 2131446995, 1666762648, 3442479324,
        2666706510, 2798801094, 2109287898, 3214720535, 485001330,  1282137225,
        2780762335, 1399541732, 3797761549, 2952997622, 457319711,  2163107841,
        2566759604, 4119800803, 3017758760, 3509545011, 3936668014, 2153774644,
        3052557008, 2380600997, 1020343361, 882559012,  1709625802, 2210285487,
        1754200573, 1285563284, 1420755524, 2580052264, 2927318343, 4005340529,
        756358275,  126515078,  3959252727, 2135204702, 894921791,  3756102777,
        594287227,  1738554800, 2292322820, 3675672227, 2430492503, 1741137831,
        1634059553, 2267849703, 1646790860, 719663478,  1465222277, 3717188733,
        1498095152, 369672206,  3152447002, 3469083042, 3638167882, 199777403,
        1937547526, 1164502738, 2578703913, 3743100516, 411465210,  1110315413,
        2659815289, 2112378186, 1931521761, 3572402566, 1194879607, 3268425783,
        4286441639, 3659890416, 485710199,  2808681985, 2030207543, 3861978250,
        3806451815, 3527254284, 3971099369, 2986131848, 10791062,   1668923750,
        453960871,  113636147,  794748838,  4152712148, 3025211691, 939912632,
        1594476987, 4025438953, 298626992,  122243381,  2656158162, 1709505417,
        141257869,  3905735260, 3309923572, 2344499361, 571058761,  3152516219,
        3147085457, 377072770,  2966351991, 3294417655, 3725774972, 2200942351,
        135870210,  353819990,  2538494617, 1685393146, 527476171,  2838037138,
        1952849601, 1486152721, 2460458056, 723237992,  3957727291, 2965229035,
        215653400,  2418364398, 1545366051, 3460829507, 520997278,  544264799,
        232632983,  1292503603, 3353407388, 4235794615, 3679153202, 674307289,
        2610444012, 3538617264, 3362501587, 2384172676, 2711114347, 558907752,
        2080365508, 3268752253, 902709283,  3160161431, 2853434261, 1937405423,
        2614801606, 3415647127, 3418598076, 1906067613, 328878379,  2647861153,
        2609252654, 1159382077, 2249725937, 1746187947, 657702536,  610697306,
        244717340,  4084566224, 3495769098, 794688043,  1818541946, 3782003838,
        1960964016, 2913212091, 2493258946, 2462872608, 1665475472, 2646734736,
        3679431764, 4007400808, 3579177017, 797921121,  1859624080, 2553518404,
        3262856360, 4204631539, 2731086371, 3472128872, 3789741539, 2336195914,
        2894449146, 648271492,  3080313678, 1989424904, 3668879612, 622832377,
        3249791716, 152803191,  1383830050, 4284977318, 1709489829, 1020498653,
        801245886,  3627624088, 1401266130, 956146142,  1976633192, 143925220,
        253816291,  1819889983, 343684470,  851039444,  1030362723, 2376620673,
        4173852814, 3687425297, 2584410797, 1856036828, 2097289517, 1638898338,
        2276984909, 3549910151, 1349315605, 1589397424, 673348798,  354141906,
        728268337,  643731911,  1928301742, 1671802879, 2058980664, 1544748112,
        303156775,  721025241,  1147032565, 1643403609, 1903311478, 1047486399,
        990279995,  1993356397, 3505433304, 2962849615, 823204374,  3559485739,
        2869072814, 38137819,   918575199,  3969395613, 1573058899, 3499471794,
        1094104661, 2691422938, 1512095063, 2574726945, 2223929236, 1103802842,
        707660713,  345548017,  2268186521, 2148889677, 3042450378, 451210792,
        597281734,  3923431479, 1221771469, 2240977869, 726295049,  3120549194,
        461975813,  192429079,  805425034,  1330848288, 4111871358, 731475551,
        4177216790, 634043238,  3186915309, 3311067218, 184208140,  1828413247,
        31143614,   1172018354, 3027273608, 2660786516, 2874824090, 2848218053,
        2912978754, 3031580303, 3956588713, 1973627223, 1844937670, 1875948161,
        2781097931, 2013412814, 1020600499, 2423426205, 2997616775, 1268942013,
        2843737537, 3700042976, 1701775203, 2592568874, 1881726929, 232723808,
        981996518,  1138101382, 3282688162, 2597593914, 2489713005, 61359070,
        3378444276, 3596052645, 157811376,  1184585608, 4148493958, 1135245635,
        3339769175, 2785059825, 1298677699, 3706136017, 1735104675, 732970111,
        2614027282, 3912696694, 1230867247, 3082361356, 3466629539, 3771033136,
        1212579468, 278299723,  2872633181, 2427337397, 2339875311, 1935767091,
        2159276994, 1716471303, 1390201505, 4027941777, 3213298367, 2023589225,
        1757386982, 2894555106, 1290527380, 1324023295, 3301823554, 1138437692,
        695778115,  898978164,  4098784559, 3353747461, 2567110559, 3431532870,
        3134376954, 2657012012, 3027968400, 2512785355, 3949773294, 2322894443,
        2948407492, 2975455979, 336916889,  4037993195, 3866460931, 3263001778,
        2642236688, 180641169,  3032058268, 3486266113, 3726526877, 2664388825,
        1277220239, 3282291648, 20545362,   1733533451, 2932434759, 2650074170,
        297036346,  2078090125, 3437909791, 2138767631, 3558122826, 2578855946,
        3824951954, 1046014746, 2923751899, 3533922721, 3924681730, 1886636973,
        961698784,  3408393538, 916874740,  3328826623, 2663540412, 3617355259,
        3316256305, 2783296552, 3017384224, 3485756381, 2592114870, 2458160314,
        4092816471, 188819224,  2918478070, 2645618607, 823505189,  277226137,
        1229331192, 3664452222, 4068681277, 1394936967, 3615117819, 2905272514,
        805725378,  3575193819, 571826141,  835996794,  850433629,  1866749763,
        2071716117, 2094466865, 785472615,  636388580,  3353596058, 794095636,
        2963856832, 4091960367, 1669301492, 1459743524, 1173572530, 3213184599,
        3505865742, 2113621083, 2449166983, 2447380784, 1828055630, 2666238328,
        922973798,  1417677665, 788179641,  1259398999, 3242478977, 961140267,
        2762757174, 439235722,  1711227441, 1963780670, 3130652245, 1095318124,
        3572342655, 3366591326, 2490817398, 3112595485, 3498465267, 3880842001,
        1388784100, 2971123924, 2479108135, 3379245666, 1594134139, 1650656600,
        2478160861, 1415892446, 2551490066, 525782900,  490935304,  714125347,
        1373156667, 2641779673, 1821842919, 912485729,  2407055859, 3136684194,
        3268437630, 3022002252, 169346103,  2921799375, 2939747538, 1169439628,
        1392693913, 549393657,  958365983,  1572753997, 3314627270, 1949935101,
        896802193,  1952911417, 1898295806, 2477825018, 2256695315, 1988023950,
        1654281335, 1779555694};

    // dynamically create a mt instance
    MersenneTwisterDynamicRng mt1(32, 521, 42, 4138, 1234);

    // set up a dynamically created mt by its description
    MersenneTwisterDynamicRngDescription desc = mt1.description();
    MersenneTwisterDynamicRng mt2(desc, 1234);

    // set up a dynamically created mt by a template parameter
    // (this was precomputed and should be identical to mt1 and mt2,
    // see the documentation in dynamiccreator.hpp)
    MersenneTwisterCustomRng<Mtdesc521_1> mt3(1234);

    for (Size i = 0; i < 500; ++i) {
        unsigned long x1 = mt1.nextInt32();
        unsigned long x2 = mt2.nextInt32();
        unsigned long x3 = mt3.nextInt32();
        if (x1 != dcmt_sample[i])
            BOOST_ERROR("Failed to verify random number #"
                        << i << " from dynamically created mt (" << x1
                        << ") against reference value (" << dcmt_sample[i]
                        << ")");
        if (x2 != dcmt_sample[i])
            BOOST_ERROR("Failed to verify random number #"
                        << i << " from mt set up by description (" << x2
                        << ") against reference value (" << dcmt_sample[i]
                        << ")");
        if (x3 != dcmt_sample[i])
            BOOST_ERROR("Failed to verify random number #"
                        << i << " from mt set up by template parameter (" << x3
                        << ") against reference value (" << dcmt_sample[i]
                        << ")");
    }
}

test_suite *MonteCarloMultiThreadedTest::suite() {
    test_suite *suite = BOOST_TEST_SUITE("Monte carlo multithreaded tests");

    suite->add(
        QUANTLIB_TEST_CASE(&MonteCarloMultiThreadedTest::testHestonEngine));
    suite->add(
        QUANTLIB_TEST_CASE(&MonteCarloMultiThreadedTest::testAmericanOption));
    suite->add(QUANTLIB_TEST_CASE(
        &MonteCarloMultiThreadedTest::testDynamicCreatorWrapper));

    return suite;
}
