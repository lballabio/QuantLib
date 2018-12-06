/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Sebastian Schlenkrich

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

#include "vanillalocalvolmodels.hpp"
#include "utilities.hpp"

#include <ql/types.hpp>
#include <ql/compounding.hpp>
#include <ql/time/all.hpp>
#include <ql/termstructures/yield/all.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/indexes/swap/euriborswap.hpp>

#include <ql/experimental/vanillalocalvolmodel/vanillalocalvolmodel.hpp>
#include <ql/experimental/vanillalocalvolmodel/vanillalocalvolsmilesection.hpp>
#include <ql/experimental/vanillalocalvolmodel/vanillalocalvoltermstructures.hpp>



using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

	// auxilliary data
	Period termsData[] = {
		Period( 0,Days),
		Period( 1,Years),
		Period( 2,Years),
		Period( 3,Years),
		Period( 5,Years),
		Period( 7,Years),
		Period(10,Years),
		Period(15,Years),
		Period(20,Years),
		Period(61,Years)   // avoid extrapolation issues with 30y caplets
	};
	std::vector<Period> terms(termsData, termsData+10);

	Real discRatesData[] = {
        -0.00147407	  ,
        -0.001761684  ,
        -0.001736745  ,
        -0.00119244	  ,
         0.000896055  ,
         0.003537077  ,
         0.007213824  ,
         0.011391278  ,
         0.013334611  ,
         0.013982809
	};
	std::vector<Real> discRates(discRatesData, discRatesData+10);

	Real proj3mRatesData[] = {
       -0.000483439	 ,
       -0.000578569	 ,
       -0.000383832	 ,
        0.000272656	 ,
        0.002478699	 ,
        0.005100113	 ,
        0.008750643	 ,
        0.012788095	 ,
        0.014534052	 ,
        0.014942896
	};
	std::vector<Real> proj3mRates(proj3mRatesData, proj3mRatesData+10);

	Real proj6mRatesData[] = {
        0.000233608	 ,
        0.000218862	 ,
        0.000504018	 ,
        0.001240556	 ,
        0.003554415	 ,
        0.006153921	 ,
        0.009688264	 ,
        0.013521628	 ,
        0.015136391	 ,
        0.015377704
	};
	std::vector<Real> proj6mRates(proj6mRatesData, proj6mRatesData+10);

	Handle<YieldTermStructure> getYTS(const std::vector<Period>& terms, const std::vector<Real>& rates, const Real spread = 0.0) {
		Date today = Settings::instance().evaluationDate();
		std::vector<Date> dates;
		for (Size k = 0; k < terms.size(); ++k) dates.push_back(NullCalendar().advance(today, terms[k], Unadjusted));
		std::vector<Real> ratesPlusSpread(rates);
		for (Size k = 0; k < ratesPlusSpread.size(); ++k) ratesPlusSpread[k] += spread;
		ext::shared_ptr<YieldTermStructure> ts = ext::shared_ptr<YieldTermStructure>(new InterpolatedZeroCurve<Cubic>(
			dates, ratesPlusSpread, Actual365Fixed(), NullCalendar()));
		return RelinkableHandle<YieldTermStructure>(ts);
	}


	Period swAtmExpiryData[] = {
        Period ( 1, Months),
        Period ( 2, Months),
        Period ( 3, Months),
        Period ( 6, Months),
        Period ( 9, Months),
        Period (12, Months),
        Period (18, Months),
        Period ( 2, Years),
        Period ( 3, Years),
        Period ( 4, Years),
        Period ( 5, Years),
        Period ( 6, Years),
        Period ( 7, Years),
        Period ( 8, Years),
        Period ( 9, Years),
        Period (10, Years),
        Period (15, Years),
        Period (20, Years),
        Period (25, Years),
        Period (30, Years)
	};
	std::vector<Period> swAtmExpiryTerms(swAtmExpiryData, swAtmExpiryData + 20);

	Period swAtmSwapData[] = {
        Period ( 1, Years),
        Period ( 2, Years),
        Period ( 3, Years),
        Period ( 4, Years),
        Period ( 5, Years),
        Period ( 6, Years),
        Period ( 7, Years),
        Period ( 8, Years),
        Period ( 9, Years),
        Period (10, Years),
        Period (15, Years),
        Period (20, Years),
        Period (25, Years),
        Period (30, Years)
	};
	std::vector<Period> swAtmSwapTerms(swAtmSwapData, swAtmSwapData + 14);

    Real swAtmRowData01m[] = {  0.001528,  0.002183,  0.002887,  0.003573,  0.004140,  0.004486,  0.004669,  0.004736,  0.004732,  0.004705,  0.005063,  0.005222,  0.005326,  0.005431 };
    Real swAtmRowData02m[] = {  0.001512,  0.002160,  0.002857,  0.003536,  0.004097,  0.004440,  0.004622,  0.004687,  0.004683,  0.004655,  0.004986,  0.005122,  0.005198,  0.005275 };
    Real swAtmRowData03m[] = {  0.001575,  0.002250,  0.002917,  0.003683,  0.004268,  0.004626,  0.004791,  0.004883,  0.004880,  0.004850,  0.005170,  0.005287,  0.005339,  0.005392 };
    Real swAtmRowData06m[] = {  0.001952,  0.002638,  0.003303,  0.003905,  0.004394,  0.004738,  0.004902,  0.005068,  0.005121,  0.005144,  0.005432,  0.005505,  0.005505,  0.005505 };
    Real swAtmRowData09m[] = {  0.002344,  0.002967,  0.003564,  0.004103,  0.004548,  0.004880,  0.005105,  0.005248,  0.005334,  0.005388,  0.005635,  0.005692,  0.005689,  0.005687 };
    Real swAtmRowData01y[] = {  0.002616,  0.003250,  0.003815,  0.004314,  0.004680,  0.004952,  0.005184,  0.005402,  0.005517,  0.005600,  0.005823,  0.005852,  0.005837,  0.005823 };
    Real swAtmRowData18m[] = {  0.003203,  0.003768,  0.004249,  0.004639,  0.004952,  0.005233,  0.005449,  0.005612,  0.005732,  0.005822,  0.005989,  0.005979,  0.005946,  0.005916 };
    Real swAtmRowData02y[] = {  0.003854,  0.004330,  0.004721,  0.005025,  0.005265,  0.005490,  0.005665,  0.005798,  0.005898,  0.005971,  0.006045,  0.006061,  0.006016,  0.005970 };
    Real swAtmRowData03y[] = {  0.005056,  0.005267,  0.005467,  0.005646,  0.005804,  0.005945,  0.006063,  0.006157,  0.006232,  0.006286,  0.006183,  0.006128,  0.006074,  0.006019 };
    Real swAtmRowData04y[] = {  0.005773,  0.005891,  0.005969,  0.006095,  0.006180,  0.006264,  0.006338,  0.006402,  0.006450,  0.006484,  0.006246,  0.006145,  0.006059,  0.006011 };
    Real swAtmRowData05y[] = {  0.006214,  0.006257,  0.006312,  0.006372,  0.006431,  0.006485,  0.006534,  0.006575,  0.006606,  0.006622,  0.006296,  0.006124,  0.006010,  0.005958 };
    Real swAtmRowData06y[] = {  0.006436,  0.006469,  0.006501,  0.006532,  0.006559,  0.006591,  0.006620,  0.006643,  0.006658,  0.006659,  0.006250,  0.006058,  0.005929,  0.005860 };
    Real swAtmRowData07y[] = {  0.006628,  0.006663,  0.006679,  0.006680,  0.006678,  0.006689,  0.006698,  0.006706,  0.006704,  0.006692,  0.006238,  0.005999,  0.005843,  0.005771 };
    Real swAtmRowData08y[] = {  0.006641,  0.006673,  0.006694,  0.006705,  0.006708,  0.006713,  0.006712,  0.006704,  0.006687,  0.006661,  0.006188,  0.005930,  0.005766,  0.005662 };
    Real swAtmRowData09y[] = {  0.006650,  0.006681,  0.006708,  0.006724,  0.006730,  0.006730,  0.006719,  0.006700,  0.006670,  0.006630,  0.006141,  0.005864,  0.005684,  0.005560 };
    Real swAtmRowData10y[] = {  0.006658,  0.006690,  0.006713,  0.006724,  0.006723,  0.006719,  0.006705,  0.006680,  0.006645,  0.006602,  0.006091,  0.005802,  0.005606,  0.005464 };
    Real swAtmRowData15y[] = {  0.006192,  0.006222,  0.006248,  0.006266,  0.006272,  0.006275,  0.006267,  0.006247,  0.006216,  0.006172,  0.005615,  0.005207,  0.004966,  0.004776 };
    Real swAtmRowData20y[] = {  0.005728,  0.005756,  0.005783,  0.005806,  0.005814,  0.005812,  0.005794,  0.005762,  0.005720,  0.005663,  0.005125,  0.004689,  0.004459,  0.004276 };
    Real swAtmRowData25y[] = {  0.005367,  0.005393,  0.005407,  0.005406,  0.005391,  0.005371,  0.005339,  0.005296,  0.005244,  0.005182,  0.004667,  0.004246,  0.004045,  0.003885 };
    Real swAtmRowData30y[] = {  0.005041,  0.005066,  0.005080,  0.005078,  0.005059,  0.005024,  0.004970,  0.004903,  0.004827,  0.004746,  0.004293,  0.003927,  0.003749,  0.003608 };
    
	Handle<SwaptionVolatilityStructure> getSwaptionVTS() {
		std::vector< std::vector<Real> > swaptionVols;
		swaptionVols.push_back(std::vector<Real>(swAtmRowData01m, swAtmRowData01m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData02m, swAtmRowData02m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData03m, swAtmRowData03m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData06m, swAtmRowData06m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData09m, swAtmRowData09m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData01y, swAtmRowData01y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData18m, swAtmRowData18m + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData02y, swAtmRowData02y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData03y, swAtmRowData03y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData04y, swAtmRowData04y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData05y, swAtmRowData05y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData06y, swAtmRowData06y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData07y, swAtmRowData07y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData08y, swAtmRowData08y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData09y, swAtmRowData09y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData10y, swAtmRowData10y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData15y, swAtmRowData15y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData20y, swAtmRowData20y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData25y, swAtmRowData25y + 14));
		swaptionVols.push_back(std::vector<Real>(swAtmRowData30y, swAtmRowData30y + 14));
		std::vector< std::vector< Handle<Quote> > > swaptionVolQuotes;
		for (Size i = 0; i < swaptionVols.size(); ++i) {
			std::vector< Handle<Quote> > row;
			for (Size j = 0; j < swaptionVols[i].size(); ++j) row.push_back(
				RelinkableHandle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(swaptionVols[i][j]))));
			swaptionVolQuotes.push_back(row);
		}
		ext::shared_ptr<SwaptionVolatilityStructure> tmp(new SwaptionVolatilityMatrix(TARGET(), Following, swAtmExpiryTerms, swAtmSwapTerms, swaptionVolQuotes, Actual365Fixed(),true, Normal));
		return RelinkableHandle<SwaptionVolatilityStructure>(tmp);
	}

    
    Real relStrikes[] = { -0.0200,  -0.0100,  -0.0050,  -0.0025,   0.0000,   0.0025,   0.0050,   0.0100,   0.0200   };
    Real smile01x01[] = {            0.002357, 0.001985, 0.002038, 0.002616, 0.003324, 0.004017, 0.005518, 0.008431 };
    Real smile3mx02[] = {                      0.001653, 0.001269, 0.002250, 0.003431, 0.004493, 0.006528, 0.010423 };
    Real smile02x02[] = {            0.003641, 0.003766, 0.003987, 0.004330, 0.004747, 0.005177, 0.006096, 0.008203 };
    Real smile01x05[] = {  0.003925, 0.004376, 0.004284, 0.004364, 0.004680, 0.005118, 0.005598, 0.006645, 0.008764 };
    Real smile05x05[] = {  0.005899, 0.005975, 0.006202, 0.006338, 0.006431, 0.006639, 0.006793, 0.007135, 0.007907 };
    Real smile3mx10[] = {  0.006652, 0.005346, 0.004674, 0.004583, 0.004850, 0.005431, 0.006161, 0.007743, 0.010880 };
    Real smile01x10[] = {  0.005443, 0.005228, 0.005271, 0.005398, 0.005600, 0.005879, 0.006203, 0.006952, 0.008603 };
    Real smile02x10[] = {  0.005397, 0.005492, 0.005685, 0.005821, 0.005971, 0.006167, 0.006367, 0.006818, 0.007840 };
	Real smile05x10[] = {  0.006096, 0.006234, 0.006427, 0.006541, 0.006622, 0.006821, 0.006946, 0.007226, 0.007875 };
    Real smile10x10[] = {  0.006175, 0.006353, 0.006485, 0.006582, 0.006602, 0.006850, 0.006923, 0.007097, 0.007495 };
    Real smile05x30[] = {  0.005560, 0.005660, 0.005792, 0.005871, 0.005958, 0.006147, 0.006233, 0.006458, 0.007048 };
	std::vector< std::vector< ext::shared_ptr<VanillaLocalVolModelSmileSection> > > smiles;

	ext::shared_ptr<VanillaLocalVolModelSmileSection> getSmileSection(const Period& expTerm, const Period& swapTerm, const Real* smileData, const Size smileOffset = 0) {
		// market data
		Handle<YieldTermStructure> discYTS = getYTS(terms, discRates);
		Handle<YieldTermStructure> proj6mYTS = getYTS(terms, proj6mRates);
		ext::shared_ptr<SwapIndex> index(new EuriborSwapIsdaFixA(swapTerm, proj6mYTS, discYTS));
		// dates and forward
		Calendar cal = index->fixingCalendar();
		Date today = discYTS->referenceDate();
		Date expiryDate = cal.advance(today, expTerm);
		Rate S0 = index->fixing(expiryDate);
		// data
		std::vector<Rate> relativeStrikes(relStrikes + smileOffset, relStrikes + 9);
		std::vector<Volatility> smileVolatilities(smileData, smileData + 9 - smileOffset);  // we might have less smile data than strikes
		Rate extrapolationRelativeStrike = relativeStrikes.back() + 0.05;
		Real extrapolationSlope = 0.0;
		// default optimisation parameters should be appropriate to set up the model/smile properly
		ext::shared_ptr<VanillaLocalVolModelSmileSection> smile(new VanillaLocalVolModelSmileSection(expiryDate, S0, relativeStrikes, smileVolatilities, extrapolationRelativeStrike, extrapolationSlope));
		//BOOST_TEST_MESSAGE("Smile is set up :)");
		// we check the calibration accuracy here
		Real smileTol = 2.0e-4; // 2bp is rather relaxed; mostly we would expect much better accuracy
		for (Size k = smileOffset; k < 9; ++k) {
			Real modelVol = smile->volatility(S0+relStrikes[k]);
			Real inputVol = smileData[k - smileOffset];
			Real variance = modelVol - inputVol;
			Time ttE = Actual365Fixed().yearFraction(today, expiryDate);
			Real vega = bachelierBlackFormulaStdDevDerivative(S0 + relStrikes[k], S0, inputVol*sqrt(ttE))*sqrt(ttE);
			//BOOST_TEST_MESSAGE("E: " << io::short_period(expTerm) << ", S: " << io::short_period(swapTerm)
			//	<< ", K: " << relStrikes[k] << ", M: " << modelVol << ", I: " << inputVol << ", D:" << variance	<< ", V:" << vega);
			Real tol = (relStrikes[k] == 0.0) ? (1.0e-12) : (smileTol);  // for ATM we expect to be spot on
			if (fabs(variance)>tol)
				BOOST_ERROR("VanillaLocalVolModel calibration accuracy failed for\n" 
					<< "E: " << io::short_period(expTerm) << ", S: " << io::short_period(swapTerm)
					<< ", K: " << relStrikes[k] << ", M: " << modelVol << ", I: " << inputVol << ", D:" << variance << ", V:" << vega);
		}
		return smile;
	}

	void testSwaptionVTSSmileInterpolation(
		const ext::shared_ptr<SwaptionVolatilityStructure>      swVTS,
		const ext::shared_ptr<VanillaLocalVolModelSmileSection> smile,
		const Period&                                           expTerm,
		const Period&                                           swapTerm,
		const Real                                              tolerance) {
		for (Real relStrike = -0.05; relStrike <= 0.05; relStrike += 0.001) {
			Rate fwd  = smile->atmLevel();
			Real vol1 = smile->volatility(fwd+relStrike);
			Real vol2 = swVTS->volatility(expTerm, swapTerm, fwd + relStrike);
			Real variance = vol2 - vol1;
			//BOOST_TEST_MESSAGE("K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
			if (fabs(variance)>tolerance)
				BOOST_ERROR("VanillaLocalVolSwaptionVTS interpolation failed for\n"
					<< "Exp: " << io::short_period(expTerm) << ", Swp: " << io::short_period(swapTerm)
					<< ", rK: " << relStrike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
		}
	}

}  // namespace


void VanillaLocalVolModelTest::testNormalModelBoundaryCase() {
	BOOST_TEST_MESSAGE("Testing normal model boundary case...");
	// model setting
	Time T = 10.0;
	Real S0 = 0.05;
	Real sigmaATM = 0.01;
	std::vector<Real> Sp(3), Sm(2), Mp(3), Mm(2);
	Sp[0] = 0.06, Sp[1] = 0.07, Sp[2] = 0.10;
	Mp[0] = 0.00, Mp[1] = 0.00, Mp[2] = 0.00;  // normal model has zero slope
	Sm[0] = 0.02, Sm[1] = -0.01;
	Mm[0] = 0.00, Mm[1] =  0.00;  // normal model has zero slope
	ext::shared_ptr<VanillaLocalVolModel> model(new VanillaLocalVolModel(T, S0, sigmaATM, Sp, Sm, Mp, Mm));
	// normal model should not require out-of-the-money adjusters
	// BOOST_TEST_MESSAGE("alpha-1: " << (model->alpha() - 1.0) << ", nu: " << model->nu() << "\n");
	if (fabs(model->alpha()-1.0) > 1.0e-12)
		BOOST_ERROR("VanillaLocalVolModel normal model boundary case failed with alpha-1 = " << model->alpha()-1.0 << ".\n");
	if (fabs(model->nu()) > 1.0e-12)
		BOOST_ERROR("VanillaLocalVolModel normal model boundary case failed with nu = " << model->nu() << ".\n");
	// pricing test settings
	Real relTol = 1.0e-12;
	for (Real strike = -0.02; strike < 0.13; strike += 0.01) {
		Real modelPrice = model->expectation((strike > S0), strike);
		Real referPrice = bachelierBlackFormula((strike > S0) ? (Option::Call) : (Option::Put), strike, S0, sigmaATM*sqrt(T));
		Real variance = fabs(modelPrice / referPrice - 1.0);
		//BOOST_TEST_MESSAGE("K: " << strike << ", Vm: " << modelPrice << ", Vr: " << referPrice << ", Var: " << variance << "\n");
		if (variance > relTol)
			BOOST_ERROR("VanillaLocalVolModel normal model boundary case pricing failed.\n"
				<< "T: " << T << ", S0: " << S0 << ", sigmaATM: " << sigmaATM << ", K: " << strike << "\n"
				<< "modelPrice: " << modelPrice << ", referPrice: " << referPrice << ", tol: " << relTol << "\n");
	}
}

void VanillaLocalVolModelTest::testShiftedLognormalModelBoundaryCase() {
	BOOST_TEST_MESSAGE("Testing shfted lognormal model boundary case...");       
	// model setting
	Time T = 10.0;
	Real S0 = 0.05;
	Real sigmaATM = 0.01;
	Real slpe = 0.20;
	std::vector<Real> Sp(3), Sm(2), Mp(3), Mm(2);
	Sp[0] = 0.06, Sp[1] = 0.07, Sp[2] = 0.10;
	Mp[0] = slpe, Mp[1] = slpe, Mp[2] = slpe;  // shifted lognormal model has constant slope
	Sm[0] = 0.02, Sm[1] = -0.01;
	Mm[0] = slpe, Mm[1] = slpe;  // normal model has constant slope
	ext::shared_ptr<VanillaLocalVolModel> model(new VanillaLocalVolModel(T, S0, sigmaATM, Sp, Sm, Mp, Mm));
	Real sigma0 = model->localVol(S0);
	Real shift = sigma0 / slpe - S0;   // localVol: sigma0 + m (S - S0) = m (S + [sigma0/m - S0])
	//BOOST_TEST_MESSAGE("sigma0: " << sigma0 << ", shift: " << shift << "\n");
	// shifted lognormal model should not require out-of-the-money adjusters
	//BOOST_TEST_MESSAGE("alpha-1: " << (model->alpha() - 1.0) << ", nu: " << model->nu() << "\n");
	if (fabs(model->alpha() - 1.0) > 1.0e-12)
		BOOST_ERROR("VanillaLocalVolModel shifted lognormal model boundary case failed with alpha-1 = " << model->alpha() - 1.0 << ".\n");
	if (fabs(model->nu()) > 1.0e-12)
		BOOST_ERROR("VanillaLocalVolModel shifted lognormal model boundary case failed with nu = " << model->nu() << ".\n");
	// pricing test settings
	Real relTol = 1.0e-12;
	for (Real strike = -0.02; strike < 0.13; strike += 0.01) {
		Real modelPrice = model->expectation((strike > S0), strike);
		Real referPrice = 0.0; // default
		if (strike+shift>=0.0)
			referPrice = blackFormula((strike > S0) ? (Option::Call) : (Option::Put), strike, S0, slpe*sqrt(T), 1.0, shift);
		Real variance = fabs(modelPrice / referPrice - 1.0);
		//BOOST_TEST_MESSAGE("K: " << strike << ", Vm: " << modelPrice << ", Vr: " << referPrice << ", Var: " << variance << "\n");
		if (variance > relTol)
			BOOST_ERROR("VanillaLocalVolModel shifted lognormal model boundary case pricing failed.\n"
				<< "T: " << T << ", S0: " << S0 << ", sigmaATM: " << sigmaATM << ", K: " << strike << "\n"
				<< "modelPrice: " << modelPrice << ", referPrice: " << referPrice << ", tol: " << relTol << "\n");
	}
}

void VanillaLocalVolModelTest::testSmileCalibration() {
   	BOOST_TEST_MESSAGE("Testing smile calibration to market data...");
	smiles.resize(5);   // 1y, 2y, 5y, 10y, 30y swaps
	smiles[0].push_back(getSmileSection(Period( 1, Years), Period( 1, Years), smile01x01, 1));
	smiles[1].push_back(getSmileSection(Period(3, Months), Period( 2, Years), smile3mx02, 2));
	smiles[1].push_back(getSmileSection(Period( 2, Years), Period( 2, Years), smile02x02, 1));
	smiles[2].push_back(getSmileSection(Period( 1, Years), Period( 5, Years), smile01x05, 0));
	smiles[2].push_back(getSmileSection(Period( 5, Years), Period( 5, Years), smile05x05, 0));
	smiles[3].push_back(getSmileSection(Period(3, Months), Period(10, Years), smile3mx10, 0));
	smiles[3].push_back(getSmileSection(Period( 1, Years), Period(10, Years), smile01x10, 0));
	smiles[3].push_back(getSmileSection(Period( 2, Years), Period(10, Years), smile02x10, 0));
	smiles[3].push_back(getSmileSection(Period( 5, Years), Period(10, Years), smile05x10, 0));
	smiles[3].push_back(getSmileSection(Period(10, Years), Period(10, Years), smile10x10, 0));
	smiles[4].push_back(getSmileSection(Period( 5, Years), Period(30, Years), smile05x30, 0));
}


void VanillaLocalVolModelTest::testSmileInterpolation() {
	BOOST_TEST_MESSAGE("Testing smile interpolation accross expiries and swap terms...");
	// this test case requires that testSmileCalibration() was run earlier to set up smiles
	// we interpolate 3m2y and 1y5y smile and check boundary cases
	// we collect essentials for 3m2y smile
	Date expDate1 = smiles[1][0]->exerciseDate();
	Rate fwd1     = smiles[1][0]->atmLevel();
	Real atmVol1  = smiles[1][0]->volatility(fwd1);
	ext::shared_ptr<VanillaLocalVolModelSmileSection> smile1(new VanillaLocalVolModelSmileSection(expDate1, fwd1, atmVol1, smiles[1][0], smiles[2][0], 0.0));
	for (Real strike = fwd1 - 0.05; strike <= fwd1 + 0.05; strike += 0.001) {
		Real vol1 = smiles[1][0]->volatility(strike);
		Real vol2 = smile1->volatility(strike);
		Real variance = vol2 - vol1;
		//BOOST_TEST_MESSAGE("K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
		if (fabs(variance)>1.0e-12)
			BOOST_ERROR("VanillaLocalVolModel interpolation failed for 3m2y smile (rho=0)\n"
				<< "K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
	}
	// we collect essentials for 1y5y smile
	Date expDate2 = smiles[2][0]->exerciseDate();
	Rate fwd2     = smiles[2][0]->atmLevel();
	Real atmVol2  = smiles[2][0]->volatility(fwd2);
	ext::shared_ptr<VanillaLocalVolModelSmileSection> smile2(new VanillaLocalVolModelSmileSection(expDate2, fwd2, atmVol2, smiles[1][0], smiles[2][0], 1.0));
	for (Real strike = fwd2 - 0.05; strike <= fwd2 + 0.05; strike += 0.001) {
		Real vol1 = smiles[2][0]->volatility(strike);
		Real vol2 = smile2->volatility(strike);
		Real variance = vol2 - vol1;
		//BOOST_TEST_MESSAGE("K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
		if (fabs(variance)>1.0e-12)
			BOOST_ERROR("VanillaLocalVolModel interpolation failed for 1y5y smile (rho=1)\n"
				<< "K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
	}
	// we also interpolate 2y10y and 10y10y smile and compare with calibrated 5y5y smile
	// we collect essentials for 5y5y smile
	Date expDate3 = smiles[3][3]->exerciseDate();
	Rate fwd3     = smiles[3][3]->atmLevel();
	Real atmVol3  = smiles[3][3]->volatility(fwd3);
	ext::shared_ptr<VanillaLocalVolModelSmileSection> smile3(new VanillaLocalVolModelSmileSection(expDate3, fwd3, atmVol3, smiles[3][2], smiles[3][4], 0.375));
	for (Real strike = fwd3 - 0.02; strike <= fwd3 + 0.02; strike += 0.001) {
		Real vol1 = smiles[3][3]->volatility(strike);
		Real vol2 = smile3->volatility(strike);
		Real variance = vol2 - vol1;
		//BOOST_TEST_MESSAGE("K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
		if (fabs(variance)>2.5e-4)  // 2.5bp is a fair tolerance for this out-of-the-model test
			BOOST_ERROR("VanillaLocalVolModel interpolation failed for 5y10y smile (rho=0.375)\n"
				<< "K: " << strike << ", Vs: " << vol1 << ", Vi: " << vol2 << ", Var: " << variance);
	}

}

void VanillaLocalVolModelTest::testSwaptionVTSInterpolation() {
	BOOST_TEST_MESSAGE("Testing interpolation via SwaptionVTS...");   
	// market data
	Handle<YieldTermStructure> discYTS = getYTS(terms, discRates);
	Handle<YieldTermStructure> proj6mYTS = getYTS(terms, proj6mRates);
	ext::shared_ptr<SwapIndex> index(new EuriborSwapIsdaFixA(Period(10, Years), proj6mYTS, discYTS));
	Handle<SwaptionVolatilityStructure> swAtmVTS = getSwaptionVTS();
	Period swSmileSwapData[] = {
		Period( 1, Years),
		Period( 2, Years),
		Period( 5, Years),
		Period(10, Years),
		Period(30, Years),
	};
	std::vector<Period> swSmileSwapTerms(swSmileSwapData, swSmileSwapData + 5);
	// now we can set up the volTS...
	// this requires that testSmileCalibration() was run earlier to set up smiles
	ext::shared_ptr<SwaptionVolatilityStructure> swVTS(new VanillaLocalVolSwaptionVTS(swAtmVTS, smiles, swSmileSwapTerms, index));
	//BOOST_TEST_MESSAGE("VTS set up :)");
	testSwaptionVTSSmileInterpolation(swVTS,smiles[0][0],Period(1, Years),  Period(1, Years) , 1.0e-7 );  // we relax tolerance; small variance probably due to date/period mismatch
	testSwaptionVTSSmileInterpolation(swVTS,smiles[1][0],Period(3, Months), Period(2, Years) , 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[1][1],Period(2, Years),  Period(2, Years) , 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[2][0],Period(1, Years),  Period(5, Years) , 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[2][1],Period(5, Years),  Period(5, Years) , 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[3][0],Period(3, Months), Period(10, Years), 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[3][1],Period(1, Years),  Period(10, Years), 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[3][2],Period(2, Years),  Period(10, Years), 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[3][3],Period(5, Years),  Period(10, Years), 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[3][4],Period(10, Years), Period(10, Years), 1.0e-12 );
	testSwaptionVTSSmileInterpolation(swVTS,smiles[4][0],Period(5, Years),  Period(30, Years), 1.0e-12 );
}


test_suite* VanillaLocalVolModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("VanillaLocalVolModel tests");
    suite->add(QUANTLIB_TEST_CASE(&VanillaLocalVolModelTest::testNormalModelBoundaryCase));
    suite->add(QUANTLIB_TEST_CASE(&VanillaLocalVolModelTest::testShiftedLognormalModelBoundaryCase));
    suite->add(QUANTLIB_TEST_CASE(&VanillaLocalVolModelTest::testSmileCalibration));
    suite->add(QUANTLIB_TEST_CASE(&VanillaLocalVolModelTest::testSmileInterpolation));
    suite->add(QUANTLIB_TEST_CASE(&VanillaLocalVolModelTest::testSwaptionVTSInterpolation));
    return suite;
}

