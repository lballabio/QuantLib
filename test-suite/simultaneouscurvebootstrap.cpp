/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Yue Tian

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

#include "simultaneouscurvebootstrap.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/multibootstrap.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/termstructures/yield/oisbasisratehelper.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/ibor/eonia.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/currencies/america.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/convexmonotoneinterpolation.hpp>
#include <ql/math/comparison.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <iomanip>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void SimultaneousBootstrapTest::testDependentCurveBootstrapConsistency() {
	BOOST_MESSAGE(
        "Testing consistency of independent curves bootstrap in simultaneous curve bootstrap framework...");

	SavedSettings backup;

	Settings::instance().evaluationDate() = Date(23, Oct, 2014);
	boost::shared_ptr<IborIndex> euribor(new Euribor(6*Months));
	boost::shared_ptr<OvernightIndex> eonia(new Eonia());
	//optimizer for simultaneous bootstrap
	boost::shared_ptr<MultiCurveOptimizer<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> > > 
					optimizer(new MultiCurveOptimizer<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> >(false));

	//ois curve
	std::vector<boost::shared_ptr<RateHelper> > insts_ois;
	std::vector<Date> oisDates;
	//ois quote
	Handle<Quote> o1(boost::shared_ptr<Quote>(new SimpleQuote(-0.000218)));//1y
	Handle<Quote> o2(boost::shared_ptr<Quote>(new SimpleQuote(-0.000222)));//2y
	Handle<Quote> o3(boost::shared_ptr<Quote>(new SimpleQuote( 0.000135)));//3y
	Handle<Quote> o4(boost::shared_ptr<Quote>(new SimpleQuote( 0.000799)));//4y
	Handle<Quote> o5(boost::shared_ptr<Quote>(new SimpleQuote( 0.001715)));//5y
	Handle<Quote> o6(boost::shared_ptr<Quote>(new SimpleQuote( 0.004082)));//7y
	Handle<Quote> o7(boost::shared_ptr<Quote>(new SimpleQuote( 0.007879)));//10y
	Handle<Quote> o8(boost::shared_ptr<Quote>(new SimpleQuote( 0.011333)));//15y
	Handle<Quote> o9(boost::shared_ptr<Quote>(new SimpleQuote( 0.01247)));//20y
	Handle<Quote> o10(boost::shared_ptr<Quote>(new SimpleQuote(0.01347)));//30y

	boost::shared_ptr<RateHelper> oh1(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 1*Years, o1, eonia)));
	boost::shared_ptr<RateHelper> oh2(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 2*Years, o2, eonia)));
	boost::shared_ptr<RateHelper> oh3(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 3*Years, o3, eonia)));
	boost::shared_ptr<RateHelper> oh4(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 4*Years, o4, eonia)));
	boost::shared_ptr<RateHelper> oh5(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 5*Years, o5, eonia)));
	boost::shared_ptr<RateHelper> oh6(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 7*Years, o6, eonia)));
	boost::shared_ptr<RateHelper> oh7(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 10*Years, o7, eonia)));
	boost::shared_ptr<RateHelper> oh8(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 15*Years, o8, eonia)));
	boost::shared_ptr<RateHelper> oh9(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 20*Years, o9, eonia)));
	boost::shared_ptr<RateHelper> oh10(boost::shared_ptr<RateHelper>(new OISRateHelper(2, 30*Years, o10, eonia)));
	insts_ois.push_back(oh1);
	insts_ois.push_back(oh2);
	insts_ois.push_back(oh3);
	insts_ois.push_back(oh4);
	insts_ois.push_back(oh5);
	insts_ois.push_back(oh6);
	insts_ois.push_back(oh7);
	insts_ois.push_back(oh8);
	insts_ois.push_back(oh9);
	insts_ois.push_back(oh10);
	oisDates.push_back(oh1->latestDate());
	oisDates.push_back(oh2->latestDate());
	oisDates.push_back(oh3->latestDate());
	oisDates.push_back(oh4->latestDate());
	oisDates.push_back(oh5->latestDate());
	oisDates.push_back(oh6->latestDate());
	oisDates.push_back(oh7->latestDate());
	oisDates.push_back(oh8->latestDate());
	oisDates.push_back(oh9->latestDate());
	oisDates.push_back(oh10->latestDate());

	//benchmark
	boost::shared_ptr<YieldTermStructure> yts_ois_benchmark( 
		new PiecewiseYieldCurve<ZeroYield, Linear, IterativeBootstrap>(0, TARGET(), insts_ois, Actual365Fixed()));
	yts_ois_benchmark->enableExtrapolation();
	yts_ois_benchmark->discount(1.0);  

    //This line is necessary for g++ because of template instantiation. However, under VC++ it works fine without this line. Check it! 
    boost::shared_ptr<YieldTermStructure> yts_ois( 
		new PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap>(0, TARGET(), insts_ois, Actual365Fixed(),1.0e-16, Linear()));
    yts_ois = boost::shared_ptr<YieldTermStructure> (
                  new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear, QuantLib::MultiBootstrap>(0, TARGET(), insts_ois, Actual365Fixed(),1.0e-16, Linear(),
                                    QuantLib::MultiBootstrap<QuantLib::PiecewiseYieldCurve<ZeroYield, Linear, QuantLib::MultiBootstrap> >(optimizer)));

	yts_ois->enableExtrapolation();

	//swap curve
	std::vector<boost::shared_ptr<RateHelper> > insts_swap;
	std::vector<boost::shared_ptr<RateHelper> > insts_swap2;
	std::vector<Date> swapDates;

	Handle<Quote> c1(boost::shared_ptr<Quote>(new SimpleQuote(0.00012)));//1m
	Handle<Quote> c2(boost::shared_ptr<Quote>(new SimpleQuote(0.0005)));//2m
	Handle<Quote> c3(boost::shared_ptr<Quote>(new SimpleQuote(0.00088)));//3m
	Handle<Quote> c4(boost::shared_ptr<Quote>(new SimpleQuote(0.00189)));//6m
	Handle<Quote> c5(boost::shared_ptr<Quote>(new SimpleQuote(0.00341)));//1y

	Handle<Quote> s1(boost::shared_ptr<Quote>(new SimpleQuote(0.00233)));//2y
	Handle<Quote> s2(boost::shared_ptr<Quote>(new SimpleQuote(0.00291)));//3y
	Handle<Quote> s3(boost::shared_ptr<Quote>(new SimpleQuote(0.00480)));//5y
	Handle<Quote> s4(boost::shared_ptr<Quote>(new SimpleQuote(0.00728)));//7y
	Handle<Quote> s5(boost::shared_ptr<Quote>(new SimpleQuote(0.01104)));//10
	Handle<Quote> s6(boost::shared_ptr<Quote>(new SimpleQuote(0.01513)));//15y
	Handle<Quote> s7(boost::shared_ptr<Quote>(new SimpleQuote(0.01715)));//20y
	Handle<Quote> s8(boost::shared_ptr<Quote>(new SimpleQuote(0.01836)));//30y
	Handle<Quote> s9(boost::shared_ptr<Quote>(new SimpleQuote(0.01955)));//40y

	boost::shared_ptr<RateHelper> dh1(new QuantLib::DepositRateHelper(c1, 1*Months, 2, TARGET(), ModifiedFollowing, false, Actual360()));
	boost::shared_ptr<RateHelper> dh2(new QuantLib::DepositRateHelper(c2, 2*Months, 2, TARGET(), ModifiedFollowing, false, Actual360()));
	boost::shared_ptr<RateHelper> dh3(new QuantLib::DepositRateHelper(c3, 3*Months, 2, TARGET(), ModifiedFollowing, false, Actual360()));
	boost::shared_ptr<RateHelper> dh4(new QuantLib::DepositRateHelper(c4, 6*Months, 2, TARGET(), ModifiedFollowing, false, Actual360()));
	boost::shared_ptr<RateHelper> dh5(new QuantLib::DepositRateHelper(c5, 1*Years, 2, TARGET(), ModifiedFollowing, false, Actual360()));
	
	//discounted by benchmark OIS curve
	boost::shared_ptr<RateHelper> sh1(new SwapRateHelper(s1, 2*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh2(new SwapRateHelper(s2, 3*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh3(new SwapRateHelper(s3, 5*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh4(new SwapRateHelper(s4, 7*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh5(new SwapRateHelper(s5, 10*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh6(new SwapRateHelper(s6, 15*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh7(new SwapRateHelper(s7, 20*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh8(new SwapRateHelper(s8, 30*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));
	boost::shared_ptr<RateHelper> sh9(new SwapRateHelper(s9, 40*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_benchmark)));

	//not depend on benchmark OIS Curve
	boost::shared_ptr<RateHelper> sh_s1(new SwapRateHelper(s1, 2*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s2(new SwapRateHelper(s2, 3*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s3(new SwapRateHelper(s3, 5*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s4(new SwapRateHelper(s4, 7*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s5(new SwapRateHelper(s5, 10*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s6(new SwapRateHelper(s6, 15*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s7(new SwapRateHelper(s7, 20*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s8(new SwapRateHelper(s8, 30*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh_s9(new SwapRateHelper(s9, 40*Years, TARGET(), Annual, ModifiedFollowing, Thirty360(), euribor, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	
	insts_swap.push_back(dh1);
	insts_swap.push_back(dh2);
	insts_swap.push_back(dh3);
	insts_swap.push_back(dh4);
	insts_swap.push_back(dh5);
	insts_swap.push_back(sh1);
	insts_swap.push_back(sh2);
	insts_swap.push_back(sh3);
	insts_swap.push_back(sh4);
	insts_swap.push_back(sh5);
	insts_swap.push_back(sh6);
	insts_swap.push_back(sh7);
	insts_swap.push_back(sh8);
	insts_swap.push_back(sh9);

	insts_swap2.push_back(dh1);
	insts_swap2.push_back(dh2);
	insts_swap2.push_back(dh3);
	insts_swap2.push_back(dh4);
	insts_swap2.push_back(dh5);
	insts_swap2.push_back(sh_s1);
	insts_swap2.push_back(sh_s2);
	insts_swap2.push_back(sh_s3);
	insts_swap2.push_back(sh_s4);
	insts_swap2.push_back(sh_s5);
	insts_swap2.push_back(sh_s6);
	insts_swap2.push_back(sh_s7);
	insts_swap2.push_back(sh_s8);
	insts_swap2.push_back(sh_s9);

	swapDates.push_back(dh1->latestDate());
	swapDates.push_back(dh2->latestDate());
	swapDates.push_back(dh3->latestDate());
	swapDates.push_back(dh4->latestDate());
	swapDates.push_back(dh5->latestDate());
	swapDates.push_back(sh1->latestDate());
	swapDates.push_back(sh2->latestDate());
	swapDates.push_back(sh3->latestDate());
	swapDates.push_back(sh4->latestDate());
	swapDates.push_back(sh5->latestDate());
	swapDates.push_back(sh6->latestDate());
	swapDates.push_back(sh7->latestDate());
	swapDates.push_back(sh8->latestDate());
	swapDates.push_back(sh9->latestDate());

	boost::shared_ptr<YieldTermStructure> yts_swap_benchmark(new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear>(0, TARGET(), insts_swap, Actual365Fixed()));
	yts_swap_benchmark->enableExtrapolation();
	yts_swap_benchmark->discount(1.0);

	boost::shared_ptr<YieldTermStructure> yts_swap(
        new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap>(0, TARGET(), insts_swap2, Actual365Fixed(),1.0e-16, Linear(),
		    MultiBootstrap<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> >(optimizer)));
	yts_swap->enableExtrapolation();

	Real tolerance = 1.0e-9;
	for(Size i = 0; i<oisDates.size(); i++) {
		if (std::fabs(yts_ois_benchmark->zeroRate(oisDates[i], Actual365Fixed(), Continuous).rate() -
					  yts_ois->zeroRate(oisDates[i], Actual365Fixed(), Continuous).rate()) > tolerance) {
			BOOST_ERROR(
					"\nBenchmark OIS curve not match simultaneously bootstrapped OIS curve on Date " 
					<< io::iso_date(oisDates[i])
					<< std::setprecision(12)
					<< "\n    estimated zero rate: " 
					<< yts_ois->zeroRate(oisDates[i], Actual365Fixed(), Continuous).rate()
					<< "\n    expected zero rate:  " 
					<< yts_ois_benchmark->zeroRate(oisDates[i], Actual365Fixed(), Continuous).rate());
		}
	}
	for(Size i = 0; i<swapDates.size(); i++) {
		if (std::fabs(yts_swap_benchmark->zeroRate(swapDates[i], Actual365Fixed(), Continuous).rate() -
					  yts_swap->zeroRate(swapDates[i], Actual365Fixed(), Continuous).rate()) > tolerance) {
			BOOST_ERROR(
					"\nBenchmark Swap curve not match simultaneously bootstrapped Swap curve on Date " 
					<< io::iso_date(swapDates[i])
					<< std::setprecision(12)
					<< "\n    estimated discount factor: " 
					<< yts_swap->zeroRate(swapDates[i], Actual365Fixed(), Continuous).rate()
					<< "\n    expected discount factor:  " 
					<< yts_swap_benchmark->zeroRate(swapDates[i], Actual365Fixed(), Continuous).rate());
		}
	}

	
}

void SimultaneousBootstrapTest::testSimultaneousBootstrapConsistency(){
	BOOST_MESSAGE(
        "Testing consistency of simultaneous curve bootstrap framework...");

	SavedSettings backup;

	Settings::instance().evaluationDate() = Date(23, Oct, 2014);
	RelinkableHandle<YieldTermStructure> libor3mProjtionCurve;
	boost::shared_ptr<IborIndex> liborIndex(new USDLibor(3*Months, libor3mProjtionCurve));
	boost::shared_ptr<OvernightIndex> overnightIndex(
		new OvernightIndex("OvernightIndex", 0, USDCurrency(), UnitedStates(UnitedStates::Settlement), Actual360()));

	boost::shared_ptr<MultiCurveOptimizer<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> > > 
				optimizer(new MultiCurveOptimizer<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> >());

	//ois quote
	Handle<Quote> o1m(boost::shared_ptr<Quote>(new SimpleQuote(0.00088)));//1m
	Handle<Quote> o2m(boost::shared_ptr<Quote>(new SimpleQuote(0.00088)));//2m
	Handle<Quote> o3m(boost::shared_ptr<Quote>(new SimpleQuote(0.00088)));//3m
	Handle<Quote> o4m(boost::shared_ptr<Quote>(new SimpleQuote(0.0009)));//4m
	Handle<Quote> o5m(boost::shared_ptr<Quote>(new SimpleQuote(0.00091)));//5m
	Handle<Quote> o6m(boost::shared_ptr<Quote>(new SimpleQuote(0.00092)));//6m
	Handle<Quote> o7m(boost::shared_ptr<Quote>(new SimpleQuote(0.00095)));//7m
	Handle<Quote> o8m(boost::shared_ptr<Quote>(new SimpleQuote(0.001)));//8m
	Handle<Quote> o9m(boost::shared_ptr<Quote>(new SimpleQuote(0.00109)));//9m
	Handle<Quote> o10m(boost::shared_ptr<Quote>(new SimpleQuote(0.00117)));//10m
	Handle<Quote> o11m(boost::shared_ptr<Quote>(new SimpleQuote(0.00128)));//11m
	Handle<Quote> o1y(boost::shared_ptr<Quote>(new SimpleQuote(0.00144)));//1y
	Handle<Quote> o18m(boost::shared_ptr<Quote>(new SimpleQuote(0.00278)));//18m
	Handle<Quote> o2y(boost::shared_ptr<Quote>(new SimpleQuote(0.00454)));//2y
	Handle<Quote> o3y(boost::shared_ptr<Quote>(new SimpleQuote(0.000833)));//3y
	Handle<Quote> o4y(boost::shared_ptr<Quote>(new SimpleQuote(0.01519)));//4y
	Handle<Quote> o5y(boost::shared_ptr<Quote>(new SimpleQuote(0.01615)));//5y

	//ois ibor basis 
	Handle<Quote> olb7y(boost::shared_ptr<Quote>(new SimpleQuote(0.002425)));//7y
	Handle<Quote> olb10y(boost::shared_ptr<Quote>(new SimpleQuote(0.0025125)));//10y
	Handle<Quote> olb12y(boost::shared_ptr<Quote>(new SimpleQuote(0.0025375)));//15y
	Handle<Quote> olb15y(boost::shared_ptr<Quote>(new SimpleQuote(0.002575)));//20y
	Handle<Quote> olb20y(boost::shared_ptr<Quote>(new SimpleQuote(0.0026)));//30y
	Handle<Quote> olb25y(boost::shared_ptr<Quote>(new SimpleQuote(0.0026)));//30y
	Handle<Quote> olb30y(boost::shared_ptr<Quote>(new SimpleQuote(0.0026)));//30y

	//swap
	Handle<Quote> s1y(boost::shared_ptr<Quote>(new SimpleQuote(0.00304)));//1y
	Handle<Quote> s2y(boost::shared_ptr<Quote>(new SimpleQuote(0.00646)));//2y
	Handle<Quote> s3y(boost::shared_ptr<Quote>(new SimpleQuote(0.01041)));//3y
	Handle<Quote> s4y(boost::shared_ptr<Quote>(new SimpleQuote(0.01386)));//4y
	Handle<Quote> s5y(boost::shared_ptr<Quote>(new SimpleQuote(0.01655)));//5y
	Handle<Quote> s6y(boost::shared_ptr<Quote>(new SimpleQuote(0.01869)));//6y
	Handle<Quote> s7y(boost::shared_ptr<Quote>(new SimpleQuote(0.02037)));//7y
	Handle<Quote> s8y(boost::shared_ptr<Quote>(new SimpleQuote(0.02179)));//8y
	Handle<Quote> s9y(boost::shared_ptr<Quote>(new SimpleQuote(0.02294)));//9y
	Handle<Quote> s10y(boost::shared_ptr<Quote>(new SimpleQuote(0.02396)));//10y
	Handle<Quote> s12y(boost::shared_ptr<Quote>(new SimpleQuote(0.02558)));//12y
	Handle<Quote> s15y(boost::shared_ptr<Quote>(new SimpleQuote(0.02725)));//15y
	Handle<Quote> s20y(boost::shared_ptr<Quote>(new SimpleQuote(0.02889)));//20y
	Handle<Quote> s25y(boost::shared_ptr<Quote>(new SimpleQuote(0.02969)));//25y
	Handle<Quote> s30y(boost::shared_ptr<Quote>(new SimpleQuote(0.03011)));//30y
	Handle<Quote> s40y(boost::shared_ptr<Quote>(new SimpleQuote(0.0303)));//40y
	Handle<Quote> s50y(boost::shared_ptr<Quote>(new SimpleQuote(0.0301)));//50y

	boost::shared_ptr<RateHelper> oh1m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 1*Months, o1m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh2m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 2*Months, o2m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh3m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 3*Months, o3m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh4m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 4*Months, o4m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh5m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 5*Months, o5m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh6m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 6*Months, o6m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh7m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 7*Months, o7m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh8m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 8*Months, o8m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh9m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 9*Months, o9m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh10m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 10*Months, o10m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh11m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 11*Months, o11m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh1y(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 1*Years, o1y, overnightIndex)));
	boost::shared_ptr<RateHelper> oh18m(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 18*Months, o18m, overnightIndex)));
	boost::shared_ptr<RateHelper> oh2y(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 2*Years, o2y, overnightIndex)));
	boost::shared_ptr<RateHelper> oh3y(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 3*Years, o3y, overnightIndex)));
	boost::shared_ptr<RateHelper> oh4y(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 4*Years, o4y, overnightIndex)));
	boost::shared_ptr<RateHelper> oh5y(boost::shared_ptr<RateHelper>(new QuantLib::OISRateHelper(0, 5*Years, o5y, overnightIndex)));

	//composite helper of ois libor basis and libor swap
	boost::shared_ptr<RateHelper> fobh7y(new QuantLib::FixedOISBasisRateHelper(2, 7*Years, olb7y, s7y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh10y(new QuantLib::FixedOISBasisRateHelper(2, 10*Years, olb10y, s10y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh12y(new QuantLib::FixedOISBasisRateHelper(2, 12*Years, olb12y, s12y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh15y(new QuantLib::FixedOISBasisRateHelper(2, 15*Years, olb15y, s15y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh20y(new QuantLib::FixedOISBasisRateHelper(2, 20*Years, olb20y, s20y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh25y(new QuantLib::FixedOISBasisRateHelper(2, 25*Years, olb25y, s25y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));
	boost::shared_ptr<RateHelper> fobh30y(new QuantLib::FixedOISBasisRateHelper(2, 30*Years, olb30y, s30y, QuantLib::Semiannual, ModifiedFollowing, Thirty360(), overnightIndex, Quarterly));

	std::vector<boost::shared_ptr<RateHelper> > insts_ois_composite;
	std::vector<Date> dates_ois_composite;

	insts_ois_composite.push_back(oh1m);
	insts_ois_composite.push_back(oh2m);
	insts_ois_composite.push_back(oh3m);
	insts_ois_composite.push_back(oh4m);
	insts_ois_composite.push_back(oh5m);
	insts_ois_composite.push_back(oh6m);
	insts_ois_composite.push_back(oh7m);
	insts_ois_composite.push_back(oh8m);
	insts_ois_composite.push_back(oh9m);
	insts_ois_composite.push_back(oh10m);
	insts_ois_composite.push_back(oh11m);
	insts_ois_composite.push_back(oh1y);
	insts_ois_composite.push_back(oh18m);
	insts_ois_composite.push_back(oh2y);
	insts_ois_composite.push_back(oh3y);
	insts_ois_composite.push_back(oh4y);
	insts_ois_composite.push_back(oh5y);

	insts_ois_composite.push_back(fobh7y);
	insts_ois_composite.push_back(fobh10y);
	insts_ois_composite.push_back(fobh12y);
	insts_ois_composite.push_back(fobh15y);
	insts_ois_composite.push_back(fobh20y);
	insts_ois_composite.push_back(fobh25y);
	insts_ois_composite.push_back(fobh30y);

	dates_ois_composite.push_back(oh1m->latestDate());
	dates_ois_composite.push_back(oh2m->latestDate());
	dates_ois_composite.push_back(oh3m->latestDate());
	dates_ois_composite.push_back(oh4m->latestDate());
	dates_ois_composite.push_back(oh5m->latestDate());
	dates_ois_composite.push_back(oh6m->latestDate());
	dates_ois_composite.push_back(oh7m->latestDate());
	dates_ois_composite.push_back(oh8m->latestDate());
	dates_ois_composite.push_back(oh9m->latestDate());
	dates_ois_composite.push_back(oh10m->latestDate());
	dates_ois_composite.push_back(oh11m->latestDate());
	dates_ois_composite.push_back(oh1y->latestDate());
	dates_ois_composite.push_back(oh18m->latestDate());
	dates_ois_composite.push_back(oh2y->latestDate());
	dates_ois_composite.push_back(oh3y->latestDate());
	dates_ois_composite.push_back(oh4y->latestDate());
	dates_ois_composite.push_back(oh5y->latestDate());

	dates_ois_composite.push_back(fobh7y->latestDate());
	dates_ois_composite.push_back(fobh10y->latestDate());
	dates_ois_composite.push_back(fobh12y->latestDate());
	dates_ois_composite.push_back(fobh15y->latestDate());
	dates_ois_composite.push_back(fobh20y->latestDate());
	dates_ois_composite.push_back(fobh25y->latestDate());
	dates_ois_composite.push_back(fobh30y->latestDate());
	//benchmark curve
	boost::shared_ptr<YieldTermStructure> yts_ois_composite( 
		new PiecewiseYieldCurve<ZeroYield, Linear>(0, UnitedStates(UnitedStates::Settlement), insts_ois_composite, Actual365Fixed()));
	yts_ois_composite->enableExtrapolation();
	yts_ois_composite->discount(1.0);

	boost::shared_ptr<IborIndex> liborIndex_tmp(new USDLibor(3*Months));
	//swap rate helper depending on yts ois composite curve
	boost::shared_ptr<RateHelper> sh1y_composite(new SwapRateHelper(s1y, 1*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh2y_composite(new SwapRateHelper(s2y, 2*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh3y_composite(new SwapRateHelper(s3y, 3*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh4y_composite(new SwapRateHelper(s4y, 4*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh5y_composite(new SwapRateHelper(s5y, 5*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh6y_composite(new SwapRateHelper(s6y, 6*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh7y_composite(new SwapRateHelper(s7y, 7*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh8y_composite(new SwapRateHelper(s8y, 8*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh9y_composite(new SwapRateHelper(s9y, 9*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh10y_composite(new SwapRateHelper(s10y, 10*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh12y_composite(new SwapRateHelper(s12y, 12*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh15y_composite(new SwapRateHelper(s15y, 15*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh20y_composite(new SwapRateHelper(s20y, 20*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh25y_composite(new SwapRateHelper(s25y, 25*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh30y_composite(new SwapRateHelper(s30y, 30*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh40y_composite(new SwapRateHelper(s40y, 40*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));
	boost::shared_ptr<RateHelper> sh50y_composite(new SwapRateHelper(s50y, 50*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex_tmp, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois_composite)));

	std::vector<boost::shared_ptr<RateHelper> > insts_swap_composite;
	std::vector<Date> dates_swap_composite;

	insts_swap_composite.push_back(sh1y_composite);
	insts_swap_composite.push_back(sh2y_composite);
	insts_swap_composite.push_back(sh3y_composite);
	insts_swap_composite.push_back(sh4y_composite);
	insts_swap_composite.push_back(sh5y_composite);
	insts_swap_composite.push_back(sh6y_composite);
	insts_swap_composite.push_back(sh7y_composite);
	insts_swap_composite.push_back(sh8y_composite);
	insts_swap_composite.push_back(sh9y_composite);
	insts_swap_composite.push_back(sh10y_composite);
	insts_swap_composite.push_back(sh12y_composite);
	insts_swap_composite.push_back(sh15y_composite);
	insts_swap_composite.push_back(sh20y_composite);
	insts_swap_composite.push_back(sh25y_composite);
	insts_swap_composite.push_back(sh30y_composite);
	insts_swap_composite.push_back(sh40y_composite);
	insts_swap_composite.push_back(sh50y_composite);

	dates_swap_composite.push_back(sh1y_composite->latestDate());
	dates_swap_composite.push_back(sh2y_composite->latestDate());
	dates_swap_composite.push_back(sh3y_composite->latestDate());
	dates_swap_composite.push_back(sh4y_composite->latestDate());
	dates_swap_composite.push_back(sh5y_composite->latestDate());
	dates_swap_composite.push_back(sh6y_composite->latestDate());
	dates_swap_composite.push_back(sh7y_composite->latestDate());
	dates_swap_composite.push_back(sh8y_composite->latestDate());
	dates_swap_composite.push_back(sh9y_composite->latestDate());
	dates_swap_composite.push_back(sh10y_composite->latestDate());
	dates_swap_composite.push_back(sh12y_composite->latestDate());
	dates_swap_composite.push_back(sh15y_composite->latestDate());
	dates_swap_composite.push_back(sh20y_composite->latestDate());
	dates_swap_composite.push_back(sh25y_composite->latestDate());
	dates_swap_composite.push_back(sh30y_composite->latestDate());
	dates_swap_composite.push_back(sh40y_composite->latestDate());
	dates_swap_composite.push_back(sh50y_composite->latestDate());

	//benchmark curve
	boost::shared_ptr<YieldTermStructure> yts_swap_composite(new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear>(0, UnitedStates(UnitedStates::Settlement), insts_swap_composite, Actual365Fixed()));
	yts_swap_composite->enableExtrapolation();
	yts_swap_composite->discount(1.0);



	//ois libor basis helper
	boost::shared_ptr<RateHelper> olbh7y(new QuantLib::IBOROISBasisRateHelper(2, 7*Years, olb7y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh10y(new QuantLib::IBOROISBasisRateHelper(2, 10*Years, olb10y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh12y(new QuantLib::IBOROISBasisRateHelper(2, 12*Years, olb12y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh15y(new QuantLib::IBOROISBasisRateHelper(2, 15*Years, olb15y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh20y(new QuantLib::IBOROISBasisRateHelper(2, 20*Years, olb20y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh25y(new QuantLib::IBOROISBasisRateHelper(2, 25*Years, olb25y, liborIndex, overnightIndex));
	boost::shared_ptr<RateHelper> olbh30y(new QuantLib::IBOROISBasisRateHelper(2, 30*Years, olb30y, liborIndex, overnightIndex));

	std::vector<boost::shared_ptr<RateHelper> > insts_ois;
	std::vector<Date> dates_ois;

	insts_ois.push_back(oh1m);
	insts_ois.push_back(oh2m);
	insts_ois.push_back(oh3m);
	insts_ois.push_back(oh4m);
	insts_ois.push_back(oh5m);
	insts_ois.push_back(oh6m);
	insts_ois.push_back(oh7m);
	insts_ois.push_back(oh8m);
	insts_ois.push_back(oh9m);
	insts_ois.push_back(oh10m);
	insts_ois.push_back(oh11m);
	insts_ois.push_back(oh1y);
	insts_ois.push_back(oh18m);
	insts_ois.push_back(oh2y);
	insts_ois.push_back(oh3y);
	insts_ois.push_back(oh4y);
	insts_ois.push_back(oh5y);

	insts_ois.push_back(olbh7y);
	insts_ois.push_back(olbh10y);
	insts_ois.push_back(olbh12y);
	insts_ois.push_back(olbh15y);
	insts_ois.push_back(olbh20y);
	insts_ois.push_back(olbh25y);
	insts_ois.push_back(olbh30y);

	dates_ois.push_back(oh1m->latestDate());
	dates_ois.push_back(oh2m->latestDate());
	dates_ois.push_back(oh3m->latestDate());
	dates_ois.push_back(oh4m->latestDate());
	dates_ois.push_back(oh5m->latestDate());
	dates_ois.push_back(oh6m->latestDate());
	dates_ois.push_back(oh7m->latestDate());
	dates_ois.push_back(oh8m->latestDate());
	dates_ois.push_back(oh9m->latestDate());
	dates_ois.push_back(oh10m->latestDate());
	dates_ois.push_back(oh11m->latestDate());
	dates_ois.push_back(oh1y->latestDate());
	dates_ois.push_back(oh18m->latestDate());
	dates_ois.push_back(oh2y->latestDate());
	dates_ois.push_back(oh3y->latestDate());
	dates_ois.push_back(oh4y->latestDate());
	dates_ois.push_back(oh5y->latestDate());

	dates_ois.push_back(olbh7y->latestDate());
	dates_ois.push_back(olbh10y->latestDate());
	dates_ois.push_back(olbh12y->latestDate());
	dates_ois.push_back(olbh15y->latestDate());
	dates_ois.push_back(olbh20y->latestDate());
	dates_ois.push_back(olbh25y->latestDate());
	dates_ois.push_back(olbh30y->latestDate());

	boost::shared_ptr<YieldTermStructure> yts_ois(
		new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap>(0, UnitedStates(UnitedStates::Settlement), insts_ois, Actual365Fixed(),1.0e-16, Linear(),
																			MultiBootstrap<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> >(optimizer)));
	yts_ois->enableExtrapolation();

	//swap rate helper
	boost::shared_ptr<RateHelper> sh1y(new SwapRateHelper(s1y, 1*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh2y(new SwapRateHelper(s2y, 2*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh3y(new SwapRateHelper(s3y, 3*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh4y(new SwapRateHelper(s4y, 4*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh5y(new SwapRateHelper(s5y, 5*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh6y(new SwapRateHelper(s6y, 6*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh7y(new SwapRateHelper(s7y, 7*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh8y(new SwapRateHelper(s8y, 8*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh9y(new SwapRateHelper(s9y, 9*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh10y(new SwapRateHelper(s10y, 10*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh12y(new SwapRateHelper(s12y, 12*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh15y(new SwapRateHelper(s15y, 15*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh20y(new SwapRateHelper(s20y, 20*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh25y(new SwapRateHelper(s25y, 25*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh30y(new SwapRateHelper(s30y, 30*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh40y(new SwapRateHelper(s40y, 40*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));
	boost::shared_ptr<RateHelper> sh50y(new SwapRateHelper(s50y, 50*Years, UnitedStates(UnitedStates::Settlement), Semiannual, ModifiedFollowing, Thirty360(), liborIndex, Handle<Quote>(), 0*Days, Handle<YieldTermStructure>(yts_ois)));

	std::vector<boost::shared_ptr<RateHelper> > insts_swap;
	std::vector<Date> dates_swap;

	insts_swap.push_back(sh1y);
	insts_swap.push_back(sh2y);
	insts_swap.push_back(sh3y);
	insts_swap.push_back(sh4y);
	insts_swap.push_back(sh5y);
	insts_swap.push_back(sh6y);
	insts_swap.push_back(sh7y);
	insts_swap.push_back(sh8y);
	insts_swap.push_back(sh9y);
	insts_swap.push_back(sh10y);
	insts_swap.push_back(sh12y);
	insts_swap.push_back(sh15y);
	insts_swap.push_back(sh20y);
	insts_swap.push_back(sh25y);
	insts_swap.push_back(sh30y);
	insts_swap.push_back(sh40y);
	insts_swap.push_back(sh50y);

	dates_swap.push_back(sh1y->latestDate());
	dates_swap.push_back(sh2y->latestDate());
	dates_swap.push_back(sh3y->latestDate());
	dates_swap.push_back(sh4y->latestDate());
	dates_swap.push_back(sh5y->latestDate());
	dates_swap.push_back(sh6y->latestDate());
	dates_swap.push_back(sh7y->latestDate());
	dates_swap.push_back(sh8y->latestDate());
	dates_swap.push_back(sh9y->latestDate());
	dates_swap.push_back(sh10y->latestDate());
	dates_swap.push_back(sh12y->latestDate());
	dates_swap.push_back(sh15y->latestDate());
	dates_swap.push_back(sh20y->latestDate());
	dates_swap.push_back(sh25y->latestDate());
	dates_swap.push_back(sh30y->latestDate());
	dates_swap.push_back(sh40y->latestDate());
	dates_swap.push_back(sh50y->latestDate());

	boost::shared_ptr<YieldTermStructure> yts_swap(
        new QuantLib::PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap>(0, UnitedStates(UnitedStates::Settlement), insts_swap, Actual365Fixed(),1.0e-16, 
                                        Linear(), MultiBootstrap<PiecewiseYieldCurve<ZeroYield, Linear, MultiBootstrap> >(optimizer)));
	
	//to tell libor ois swap to use yts_swap curve as projection curve
	libor3mProjtionCurve.linkTo(yts_swap);

	Real tolerance = 1.0e-9;
	for(Size i = 0; i<dates_ois_composite.size(); i++) {
		if (std::fabs(yts_ois_composite->zeroRate(dates_ois_composite[i], Actual365Fixed(), Continuous).rate() -
					  yts_ois->zeroRate(dates_ois_composite[i], Actual365Fixed(), Continuous).rate()) > tolerance) {
			BOOST_ERROR(
					"\nOIS curve from composte rate helper not match simultaneously bootstrapped OIS curve on Date " 
					<< io::iso_date(dates_ois[i])
					<< std::setprecision(12)
					<< "\n    estimated zero rate: " 
					<< yts_ois->zeroRate(dates_ois[i], Actual365Fixed(), Continuous).rate()
					<< "\n    expected zero rate:  " 
					<< yts_ois_composite->zeroRate(dates_ois[i], Actual365Fixed(), Continuous).rate());
		}
	}
	for(Size i = 0; i<dates_swap.size(); i++) {
		if (std::fabs(yts_swap_composite->zeroRate(dates_swap_composite[i], Actual365Fixed(), Continuous).rate() -
					  yts_swap->zeroRate(dates_swap_composite[i], Actual365Fixed(), Continuous).rate()) > tolerance) {
			BOOST_ERROR(
					"\nSwap curve from composte rate helper not match simultaneously bootstrapped Swap curve on Date " 
					<< io::iso_date(dates_swap[i])
					<< std::setprecision(12)
					<< "\n    estimated zero rate: " 
					<< yts_swap->zeroRate(dates_swap[i], Actual365Fixed(), Continuous).rate()
					<< "\n    expected zero rate:  " 
					<< yts_swap_composite->zeroRate(dates_swap[i], Actual365Fixed(), Continuous).rate());
		}
	}
}
    


test_suite* SimultaneousBootstrapTest::suite() {

    test_suite* suite = BOOST_TEST_SUITE("Simultaneous bootstrap of Piecewise yield curves tests");

    suite->add(QUANTLIB_TEST_CASE(
                 &SimultaneousBootstrapTest::testDependentCurveBootstrapConsistency));
	suite->add(QUANTLIB_TEST_CASE(
                 &SimultaneousBootstrapTest::testSimultaneousBootstrapConsistency));


    return suite;
}


