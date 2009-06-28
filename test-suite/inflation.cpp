/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2008 Piero Del Boca

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

#include "inflation.hpp"
#include "utilities.hpp"
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/schedule.hpp>

using boost::unit_test_framework::test_suite;


using namespace QuantLib;

namespace {

    struct Datum {
        Date date;
        Rate rate;
    };

    boost::shared_ptr<YieldTermStructure> nominalTermStructure() {
        Date evaluationDate(13, August, 2007);
        return boost::shared_ptr<YieldTermStructure>(
                          new FlatForward(evaluationDate, 0.05, Actual360()));
    }

    template <class T, class U>
    std::vector<boost::shared_ptr<BootstrapHelper<T> > > makeHelpers(
                     Datum iiData[], Size N, const Period &lag,
                     Integer settlementDays, const Calendar &calendar,
                     const BusinessDayConvention &bdc,
                     const DayCounter &dc, Frequency frequency,
                     const boost::shared_ptr<YieldTermStructure> &nominalTS) {

        std::vector<boost::shared_ptr<BootstrapHelper<T> > > instruments;
        for (Size i=0; i<N; i++) {
            Date maturity = iiData[i].date;
            Handle<Quote> quote(boost::shared_ptr<Quote>(
                                      new SimpleQuote(iiData[i].rate/100.0)));
            boost::shared_ptr<BootstrapHelper<T> > anInstrument(new U(
                                         quote, lag, maturity, settlementDays,
                                         calendar, bdc, dc, frequency));
            instruments.push_back(anInstrument);
        }

        return instruments;
    }

}


void InflationTest::testYoYTermStructure() {
    BOOST_MESSAGE("Testing year-on-year inflation term structure...");

    SavedSettings backup;

    // try the YY UK
    Calendar calendar = UnitedKingdom();
    BusinessDayConvention bdc = ModifiedFollowing;
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
                                         .withTenor(1*Months)
                                         .withCalendar(UnitedKingdom())
                                         .withConvention(ModifiedFollowing);
    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
                       192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
                       194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
                       198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                       202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
                       207.3, -999.0, -999 };

    RelinkableHandle<YoYInflationTermStructure> hy;
    bool interp = true;
    boost::shared_ptr<YYUKRPIr> iir(new YYUKRPIr(Monthly, false, interp, hy));
    for (Size i=0; i<rpiSchedule.size();i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
    }

    boost::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

    // now build the YoY inflation curve
    Datum yyData[] = {
        { Date(13, August, 2008), 2.95 },
        { Date(13, August, 2009), 2.95 },
        { Date(13, August, 2010), 2.93 },
        { Date(15, August, 2011), 2.955 },
        { Date(13, August, 2012), 2.945 },
        { Date(13, August, 2013), 2.985 },
        { Date(13, August, 2014), 3.01 },
        { Date(13, August, 2015), 3.035 },
        { Date(15, August, 2016), 3.055 },
        { Date(14, August, 2017), 3.075 },
        { Date(13, August, 2019), 3.105 },
        { Date(15, August, 2022), 3.135 },
        { Date(13, August, 2027), 3.155 },
        { Date(13, August, 2032), 3.145 },
        { Date(13, August, 2037), 3.145 }
    };

    Period lag = Period(2,Months);
    Integer settlementDays = 0;
    DayCounter dc = Thirty360();
    Frequency frequency = Monthly;
    std::vector<boost::shared_ptr<BootstrapHelper<YoYInflationTermStructure> > > helpers =
        makeHelpers<YoYInflationTermStructure, YyiisInflationHelper>(
                                                       yyData, LENGTH(yyData),
                                                       lag, settlementDays,
                                                       calendar, bdc, dc,
                                                       frequency, nominalTS);

    Date d = evaluationDate - lag;
    Rate baseYoYRate = iir->fixing(d);
    boost::shared_ptr< PiecewiseYoYInflationCurve<Linear> > pYITS(
            new PiecewiseYoYInflationCurve<Linear>(
                    evaluationDate, calendar, dc, lag, frequency, baseYoYRate,
                    Handle<YieldTermStructure>(nominalTS), helpers));
    pYITS->recalculate();

    const Real eps = 0.00001;

    for (Size i=0; i<helpers.size(); i++) {
        if (std::fabs(helpers[i]->impliedQuote()-yyData[i].rate/100.0) > eps)
            BOOST_ERROR("could not reprice " << io::ordinal(i+1) << " helper"
                        << ", data " << yyData[i].rate/100.0
                        << ", implied quote " << helpers[i]->impliedQuote());
    }


    // Test the forecasting capability of the index.
    hy.linkTo(pYITS);

    from = pYITS->baseDate();
    to = pYITS->maxDate()-1*Months; // a bit of margin for adjustments
    Schedule testIndex = MakeSchedule().from(from).to(to)
                                       .withTenor(1*Months)
                                       .withCalendar(UnitedKingdom())
                                       .withConvention(ModifiedFollowing);

    Date bd = hy->baseDate();
    // get the relevant period end
    std::pair<Date,Date> lim = inflationPeriod(bd,hy->frequency());
    Date trueBaseDate = lim.second;
    for (Size i=0; i<testIndex.size();i++) {
        Date d = testIndex[i];
        Real calc = pYITS->yoyRate(d);
        if (d<=trueBaseDate)
            calc = iir->fixing(d,false);  // still historical
        if (std::fabs(calc - iir->fixing(d,true)) > eps)
            BOOST_ERROR("YY index does not forecast correctly for date " << d
                        << ", correct:  " << pYITS->yoyRate(d)
                        << ", fix: " << iir->fixing(d,true));
    }
	
	//Testing multiplicative _price_ seasonality: this should have no effect on year-on-year rates
	//when exactly one year of factors is given, but should have an effect for multi-year seasonality.
	
	Date seasonalityBaseDate(31,January,2007);
		
	//Creating two different seasonality objects, constant and real then show that they create NO difference for YoY
	//
	boost::shared_ptr<MultiplicativePriceSeasonality> seasonality_const(new MultiplicativePriceSeasonality());
	std::vector<Rate> seasonalityFactors_const(12, 1.0);
	seasonality_const->set(seasonalityBaseDate,Monthly,seasonalityFactors_const);
	Rate fixing_const[] = {
		iir->fixing(Date(14,July     ,2007),true),
		iir->fixing(Date(14,August   ,2007),true),
		iir->fixing(Date(14,September,2007),true),
		iir->fixing(Date(14,October  ,2007),true),
		iir->fixing(Date(14,November ,2007),true),
		iir->fixing(Date(14,December ,2007),true),
		iir->fixing(Date(14,January  ,2008),true),
		iir->fixing(Date(14,February ,2008),true),
		iir->fixing(Date(14,March    ,2008),true),
		iir->fixing(Date(14,April    ,2008),true),
		iir->fixing(Date(14,May      ,2008),true),
		iir->fixing(Date(14,June     ,2008),true)
	};
	
	std::vector<Rate> seasonalityFactors_1Y(12);
	seasonalityFactors_1Y[0] = 1.003245;
	seasonalityFactors_1Y[1] = 1.000000; 
	seasonalityFactors_1Y[2] = 0.999715; 
	seasonalityFactors_1Y[3] = 1.000495;
	seasonalityFactors_1Y[4] = 1.000929;
	seasonalityFactors_1Y[5] = 0.998687;
	seasonalityFactors_1Y[6] = 0.995949;
	seasonalityFactors_1Y[7] = 0.994682;
	seasonalityFactors_1Y[8] = 0.995949;
	seasonalityFactors_1Y[9] = 1.000519;
	seasonalityFactors_1Y[10] = 1.003705;
	seasonalityFactors_1Y[11] = 1.004186;
	boost::shared_ptr<MultiplicativePriceSeasonality> seasonality_1Y(
		new MultiplicativePriceSeasonality(seasonalityBaseDate,Monthly,seasonalityFactors_1Y));
    hy->setSeasonality(seasonality_1Y);
    QL_REQUIRE(hy->hasSeasonality(),"[0] incorrectly believes NO seasonality correction");
	
	Rate seasonalityFixing_1Y[] = {
		iir->fixing(Date(14,July     ,2007),true),
		iir->fixing(Date(14,August   ,2007),true),
		iir->fixing(Date(14,September,2007),true),
		iir->fixing(Date(14,October  ,2007),true),
		iir->fixing(Date(14,November ,2007),true),
		iir->fixing(Date(14,December ,2007),true),
		iir->fixing(Date(14,January  ,2008),true),
		iir->fixing(Date(14,February ,2008),true),
		iir->fixing(Date(14,March    ,2008),true),
		iir->fixing(Date(14,April    ,2008),true),
		iir->fixing(Date(14,May      ,2008),true),
		iir->fixing(Date(14,June     ,2008),true)
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(fixing_const[i] - seasonalityFixing_1Y[i]) > eps) {
			BOOST_ERROR("Seasonality doesn't work correctly when seasonality factors are set = 1");
		}
	}
	
	//Creating multi-year price seasonality and show that this does have an effect for YoY
	//
	std::vector<Rate> seasonalityFactors_2Y(24);
	seasonalityFactors_2Y[0] = 1.003245;
	seasonalityFactors_2Y[1] = 1.000000;
	seasonalityFactors_2Y[2] = 0.999715; 
	seasonalityFactors_2Y[3] = 1.000495;
	seasonalityFactors_2Y[4] = 1.000929;
	seasonalityFactors_2Y[5] = 0.998687;	// first June, curve start month
	seasonalityFactors_2Y[6] = 0.995949;
	seasonalityFactors_2Y[7] = 0.994682;
	seasonalityFactors_2Y[8] = 0.995949;
	seasonalityFactors_2Y[9] = 1.000519;
	seasonalityFactors_2Y[10] = 1.003705;
	seasonalityFactors_2Y[11] = 1.004186;
	seasonalityFactors_2Y[12] = 1.006245;
	seasonalityFactors_2Y[13] = 1.000000;
	seasonalityFactors_2Y[14] = 0.999015; 
	seasonalityFactors_2Y[15] = 1.000895;
	seasonalityFactors_2Y[16] = 1.001829;
	seasonalityFactors_2Y[17] = 0.998687;	// should be same as first June, this is curve start month
	seasonalityFactors_2Y[18] = 0.994949;
	seasonalityFactors_2Y[19] = 0.993682;
	seasonalityFactors_2Y[20] = 0.994949;
	seasonalityFactors_2Y[21] = 1.001019;
	seasonalityFactors_2Y[22] = 1.006705;
	seasonalityFactors_2Y[23] = 1.008186;
	boost::shared_ptr<MultiplicativePriceSeasonality> seasonality_2Y(
		new MultiplicativePriceSeasonality(seasonalityBaseDate,Monthly,seasonalityFactors_2Y));
	
	// remove seasonality and get base values, and check; recall that the seasonality base is January 2007
	// - for multi-year seasonality the actual base year is important
    hy->setSeasonality();
    QL_REQUIRE(!hy->hasSeasonality(),"[01] incorrectly believes IS seasonality correction");
	
	 Rate expectedFixing[] = {
	 (iir->fixing(Date(14,July     ,2007),true) + 1) * 0.995949 /0.994949  -1,
	 (iir->fixing(Date(14,August   ,2007),true) + 1) * 0.994682 /0.993682  -1,
	 (iir->fixing(Date(14,September,2007),true) + 1) * 0.995949 /0.994949  -1,
	 (iir->fixing(Date(14,October  ,2007),true) + 1) * 1.000519 /1.001019  -1,
	 (iir->fixing(Date(14,November ,2007),true) + 1) * 1.003705 /1.006705  -1,
	 (iir->fixing(Date(14,December ,2007),true) + 1) * 1.004186 /1.008186  -1,
	 (iir->fixing(Date(14,January  ,2008),true) + 1) * 1.006245 /1.003245  -1,
	 (iir->fixing(Date(14,February ,2008),true) + 1) * 1.000000 /1.000000  -1,   
	 (iir->fixing(Date(14,March    ,2008),true) + 1) * 0.999015 /0.999715  -1,
	 (iir->fixing(Date(14,April    ,2008),true) + 1) * 1.000895 /1.000495  -1,
	 (iir->fixing(Date(14,May      ,2008),true) + 1) * 1.001829 /1.000929  -1,
	 (iir->fixing(Date(14,June     ,2008),true) + 1) * 0.998687 /0.998687  -1
	 };
	 
	// set seasonality to get multi-year values
	hy->setSeasonality(seasonality_2Y);
    QL_REQUIRE(hy->hasSeasonality(),"[02] incorrectly believes NO seasonality correction");
	Rate seasonalityFixing_real[] = {
		iir->fixing(Date(14,July     ,2007),true),
		iir->fixing(Date(14,August   ,2007),true),
		iir->fixing(Date(14,September,2007),true),
		iir->fixing(Date(14,October  ,2007),true),
		iir->fixing(Date(14,November ,2007),true),
		iir->fixing(Date(14,December ,2007),true),
		iir->fixing(Date(14,January  ,2008),true),
		iir->fixing(Date(14,February ,2008),true),
		iir->fixing(Date(14,March    ,2008),true),
		iir->fixing(Date(14,April    ,2008),true),
		iir->fixing(Date(14,May      ,2008),true),
		iir->fixing(Date(14,June     ,2008),true)
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(expectedFixing[i] - seasonalityFixing_real[i]) > eps) {
			BOOST_ERROR("Seasonality doesn't work correctly when considering multi-year seasonality factors != 1");
		}
	}
	
	//Testing Unset function
	//
    QL_REQUIRE(hy->hasSeasonality(),"[1] incorrectly believes NO seasonality correction");
	hy->setSeasonality();
    QL_REQUIRE(!hy->hasSeasonality(),"[2] incorrectly believes HAS seasonality correction");
	
	Rate seasonalityFixing_unset[] = {
		iir->fixing(Date(14,July     ,2007),true),
		iir->fixing(Date(14,August   ,2007),true),
		iir->fixing(Date(14,September,2007),true),
		iir->fixing(Date(14,October  ,2007),true),
		iir->fixing(Date(14,November ,2007),true),
		iir->fixing(Date(14,December ,2007),true),
		iir->fixing(Date(14,January  ,2008),true),
		iir->fixing(Date(14,February ,2008),true),
		iir->fixing(Date(14,March    ,2008),true),
		iir->fixing(Date(14,April    ,2008),true),
		iir->fixing(Date(14,May      ,2008),true),
		iir->fixing(Date(14,June     ,2008),true)
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(seasonalityFixing_unset[i] - fixing_const[i]) > eps) {
			BOOST_ERROR("UnsetSeasonality doesn't work correctly");
		}
	}
}


void InflationTest::testZeroTermStructure() {
    BOOST_MESSAGE("Testing zero inflation term structure...");
	
    SavedSettings backup;

    // try the Zero UK
    Calendar calendar = UnitedKingdom();
    BusinessDayConvention bdc = ModifiedFollowing;
    Date evaluationDate(13, August, 2007);
    evaluationDate = calendar.adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
                                         .withTenor(1*Months)
                                         .withCalendar(UnitedKingdom())
                                         .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
                       192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
                       194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
                       198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                       202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
                       207.3, -999.0, -999.0 };

    RelinkableHandle<ZeroInflationTermStructure> hz;
    bool interp = true;
    boost::shared_ptr<UKRPI> ii(new UKRPI(Monthly, false, interp, hz));
    for (Size i=0; i<rpiSchedule.size();i++) {
        ii->addFixing(rpiSchedule[i], fixData[i]);
    }

    boost::shared_ptr<YieldTermStructure> nominalTS = nominalTermStructure();

    // now build the zero inflation curve
    Datum zcData[] = {
        { Date(13, August, 2008), 2.93 },
        { Date(13, August, 2009), 2.95 },
        { Date(13, August, 2010), 2.965 },
        { Date(15, August, 2011), 2.98 },
        { Date(13, August, 2012), 3.0 },
        { Date(13, August, 2014), 3.06 },
        { Date(13, August, 2017), 3.175 },
        { Date(13, August, 2019), 3.243 },
        { Date(15, August, 2022), 3.293 },
        { Date(14, August, 2027), 3.338 },
        { Date(13, August, 2032), 3.348 },
        { Date(15, August, 2037), 3.348 },
        { Date(13, August, 2047), 3.308 },
        { Date(13, August, 2057), 3.228 }
    };

    Period lag = Period(2,Months);
    Integer settlementDays = 0;
    DayCounter dc = Thirty360();
    Frequency frequency = Monthly;
    std::vector<boost::shared_ptr<BootstrapHelper<ZeroInflationTermStructure> > > helpers =
        makeHelpers<ZeroInflationTermStructure,ZciisInflationHelper>(
                                                  zcData, LENGTH(zcData),
                                                  lag, settlementDays,
                                                  calendar, bdc, dc, frequency,
                                                  nominalTS);

    Date d = evaluationDate - lag;
    Rate baseZeroRate = (ii->fixing(d) / ii->fixing(d - 1*Years)) - 1.0;
    boost::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZITS(
        new PiecewiseZeroInflationCurve<Linear>(
                   evaluationDate, calendar, dc, lag, frequency, baseZeroRate,
                   Handle<YieldTermStructure>(nominalTS), helpers));
    pZITS->recalculate();

    const Real eps = 0.000001;

    for (Size i=0; i<helpers.size(); i++) {
        if (std::fabs(helpers[i]->impliedQuote()-zcData[i].rate/100.0) > eps)
            BOOST_ERROR("could not reprice " << io::ordinal(i+1) << " helper"
                        << ", data " << zcData[i].rate/100.0
                        << ", implied quote " << helpers[i]->impliedQuote());
    }

    // Here we test the forecasting capability of the index.
    hz.linkTo(pZITS);

    from = hz->baseDate();
    to = hz->maxDate()-1*Months; // a bit of margin for adjustments
    Schedule testIndex = MakeSchedule().from(from).to(to)
                                       .withTenor(1*Months)
                                       .withCalendar(UnitedKingdom())
                                       .withConvention(ModifiedFollowing);

    Date bd = hz->baseDate();
    // get the relevant period end
    std::pair<Date,Date> lim = inflationPeriod(bd,hz->frequency());
    Date trueBaseDate = lim.second;
    Real bf = ii->fixing(bd);
    for (Size i=0; i<testIndex.size();i++) {
        Date d = testIndex[i];
        Real z = hz->zeroRate(d);
        Real t = hz->dayCounter().yearFraction(trueBaseDate,d);
        Real calc = bf * pow( 1+z, t);
        if (t<=0)
            calc = ii->fixing(d,false); // still historical
        if (std::fabs(calc - ii->fixing(d,true)) > eps)
            BOOST_ERROR("ZC index does not forecast correctly for date " << d
                        << ", correct:  " << calc
                        << ", fix: " << ii->fixing(d,true));
    }
	
	//Testing multiplicative seasonality in price
	//
	
	//Seasonality factors NOT normalized
	//
	Date seasonallityBaseDate(31,January,trueBaseDate.year());
	std::vector<Rate> seasonalityFactors(12);
	seasonalityFactors[0] = 1.003245;
	seasonalityFactors[1] = 1.000000;
	seasonalityFactors[2] = 0.999715; 
	seasonalityFactors[3] = 1.000495;
	seasonalityFactors[4] = 1.000929;
	seasonalityFactors[5] = 0.998687;
	seasonalityFactors[6] = 0.995949;
	seasonalityFactors[7] = 0.994682;
	seasonalityFactors[8] = 0.995949;
	seasonalityFactors[9] = 1.000519;
	seasonalityFactors[10] = 1.003705;
	seasonalityFactors[11] = 1.004186;
	
	//Creating two different seasonality objects
	//
	boost::shared_ptr<MultiplicativePriceSeasonality> seasonality_1(new MultiplicativePriceSeasonality());
	std::vector<Rate> seasonalityFactors_1(12, 1.0);
	seasonality_1->set(seasonallityBaseDate,Monthly,seasonalityFactors_1);
	
	boost::shared_ptr<MultiplicativePriceSeasonality> seasonality_real(
		new MultiplicativePriceSeasonality(seasonallityBaseDate,Monthly,seasonalityFactors));                                                                  
	//Testing seasonality correction when seasonality factors are = 1
	//
	Rate fixing[] = {
		ii->fixing(Date(14,January  ,2013),true),
		ii->fixing(Date(14,February ,2013),true),
		ii->fixing(Date(14,March    ,2013),true),
		ii->fixing(Date(14,April    ,2013),true),
		ii->fixing(Date(14,May      ,2013),true),
		ii->fixing(Date(14,June     ,2013),true),
		ii->fixing(Date(14,July     ,2013),true),
		ii->fixing(Date(14,August   ,2013),true),
		ii->fixing(Date(14,September,2013),true),
		ii->fixing(Date(14,October  ,2013),true),
		ii->fixing(Date(14,November ,2013),true),
		ii->fixing(Date(14,December ,2013),true)
	};
	
	hz->setSeasonality(seasonality_1);
    QL_REQUIRE(hz->hasSeasonality(),"[44] incorrectly believes NO seasonality correction");
	
	Rate seasonalityFixing_1[] = {
		ii->fixing(Date(14,January  ,2013),true),
		ii->fixing(Date(14,February ,2013),true),
		ii->fixing(Date(14,March    ,2013),true),
		ii->fixing(Date(14,April    ,2013),true),
		ii->fixing(Date(14,May      ,2013),true),
		ii->fixing(Date(14,June     ,2013),true),
		ii->fixing(Date(14,July     ,2013),true),
		ii->fixing(Date(14,August   ,2013),true),
		ii->fixing(Date(14,September,2013),true),
		ii->fixing(Date(14,October  ,2013),true),
		ii->fixing(Date(14,November ,2013),true),
		ii->fixing(Date(14,December ,2013),true)
		
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(fixing[i] - seasonalityFixing_1[i]) > eps) {
			BOOST_ERROR("Seasonality doesn't work correctly when seasonality factors are set = 1");
		}
	}
	
	//Testing seasonality correction when seasonality factors are different from 1
	//
	//0.998687 is the seasonality factor corresponding to June (the base CPI curve month)
	//
	Rate expectedFixing[] = {
		ii->fixing(Date(14,January  ,2013),true) * 1.003245/0.998687,
		ii->fixing(Date(14,February ,2013),true) * 1.000000/0.998687,   
		ii->fixing(Date(14,March    ,2013),true) * 0.999715/0.998687,
		ii->fixing(Date(14,April    ,2013),true) * 1.000495/0.998687,
		ii->fixing(Date(14,May      ,2013),true) * 1.000929/0.998687,
		ii->fixing(Date(14,June     ,2013),true) * 0.998687/0.998687,
		ii->fixing(Date(14,July     ,2013),true) * 0.995949/0.998687,
		ii->fixing(Date(14,August   ,2013),true) * 0.994682/0.998687,
		ii->fixing(Date(14,September,2013),true) * 0.995949/0.998687,
		ii->fixing(Date(14,October  ,2013),true) * 1.000519/0.998687,
		ii->fixing(Date(14,November ,2013),true) * 1.003705/0.998687,
		ii->fixing(Date(14,December ,2013),true) * 1.004186/0.998687
	};

	hz->setSeasonality(seasonality_real);
	
	Rate seasonalityFixing_real[] = {
		ii->fixing(Date(14,January  ,2013),true),
		ii->fixing(Date(14,February ,2013),true),   
		ii->fixing(Date(14,March    ,2013),true),
		ii->fixing(Date(14,April    ,2013),true),
		ii->fixing(Date(14,May      ,2013),true),
		ii->fixing(Date(14,June     ,2013),true),
		ii->fixing(Date(14,July     ,2013),true),
		ii->fixing(Date(14,August   ,2013),true),
		ii->fixing(Date(14,September,2013),true),
		ii->fixing(Date(14,October  ,2013),true),
		ii->fixing(Date(14,November ,2013),true),
		ii->fixing(Date(14,December ,2013),true)
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(expectedFixing[i] - seasonalityFixing_real[i]) > eps) {
			BOOST_ERROR("Seasonality doesn't work correctly when considering seasonality factors != 1");
		}
	}
	
	//Testing Unset function
	//
    QL_REQUIRE(hz->hasSeasonality(),"[4] incorrectly believes NO seasonality correction");
	hz->setSeasonality();
    QL_REQUIRE(!hz->hasSeasonality(),"[5] incorrectly believes HAS seasonality correction");
	
	Rate seasonalityFixing_unset[] = {
		ii->fixing(Date(14,January  ,2013),true),
		ii->fixing(Date(14,February ,2013),true),   
		ii->fixing(Date(14,March    ,2013),true),
		ii->fixing(Date(14,April    ,2013),true),
		ii->fixing(Date(14,May      ,2013),true),
		ii->fixing(Date(14,June     ,2013),true),
		ii->fixing(Date(14,July     ,2013),true),
		ii->fixing(Date(14,August   ,2013),true),
		ii->fixing(Date(14,September,2013),true),
		ii->fixing(Date(14,October  ,2013),true),
		ii->fixing(Date(14,November ,2013),true),
		ii->fixing(Date(14,December ,2013),true)
	};
	
	for(int i=0;i<12;i++){
		if(std::fabs(seasonalityFixing_unset[i] - seasonalityFixing_1[i]) > eps) {
			BOOST_ERROR("UnsetSeasonality doesn't work correctly");
		}
	}
	
	
	
	
	
}


void InflationTest::testYYIndex() {
    BOOST_MESSAGE("Testing year-on-year inflation indices...");
	
    SavedSettings backup;
	
    YYEUHICP yyeuhicp(Monthly, false, false);
    if (yyeuhicp.name() != "EU YY_HICP"
        || yyeuhicp.frequency() != Monthly
        || yyeuhicp.revised()
        || yyeuhicp.interpolated()
        || yyeuhicp.ratio()
        || yyeuhicp.availabilityLag() != 3*Months) {
        BOOST_ERROR("wrong year-on-year EU HICP data ("
                    << yyeuhicp.name() << ", "
                    << yyeuhicp.frequency() << ", "
                    << yyeuhicp.revised() << ", "
                    << yyeuhicp.interpolated() << ", "
                    << yyeuhicp.ratio() << ", "
                    << yyeuhicp.availabilityLag() << ")");
    }

    YYEUHICPr yyeuhicpr(Monthly, false, true);
    if (yyeuhicpr.name() != "EU YYR_HICP"
        || yyeuhicpr.frequency() != Monthly
        || yyeuhicpr.revised()
        || !yyeuhicpr.interpolated()
        || !yyeuhicpr.ratio()
        || yyeuhicpr.availabilityLag() != 3*Months) {
        BOOST_ERROR("wrong year-on-year EU HICPr data ("
                    << yyeuhicpr.name() << ", "
                    << yyeuhicpr.frequency() << ", "
                    << yyeuhicpr.revised() << ", "
                    << yyeuhicpr.interpolated() << ", "
                    << yyeuhicpr.ratio() << ", "
                    << yyeuhicpr.availabilityLag() << ")");
    }

    YYUKRPI yyukrpi(Monthly, true, false);
    if (yyukrpi.name() != "UK YY_RPI"
        || yyukrpi.frequency() != Monthly
        || !yyukrpi.revised()
        || yyukrpi.interpolated()
        || yyukrpi.ratio()
        || yyukrpi.availabilityLag() != 2*Months) {
        BOOST_ERROR("wrong year-on-year UK RPI data ("
                    << yyukrpi.name() << ", "
                    << yyukrpi.frequency() << ", "
                    << yyukrpi.revised() << ", "
                    << yyukrpi.interpolated() << ", "
                    << yyukrpi.ratio() << ", "
                    << yyukrpi.availabilityLag() << ")");
    }

    YYUKRPIr yyukrpir(Monthly, true, true);
    if (yyukrpir.name() != "UK YYR_RPI"
        || yyukrpir.frequency() != Monthly
        || !yyukrpir.revised()
        || !yyukrpir.interpolated()
        || !yyukrpir.ratio()
        || yyukrpir.availabilityLag() != 2*Months) {
        BOOST_ERROR("wrong year-on-year UK RPIr data ("
                    << yyukrpir.name() << ", "
                    << yyukrpir.frequency() << ", "
                    << yyukrpir.revised() << ", "
                    << yyukrpir.interpolated() << ", "
                    << yyukrpir.ratio() << ", "
                    << yyukrpir.availabilityLag() << ")");
    }


    // Retrieval test.
    //----------------
    // make sure of the evaluation date
    Date evaluationDate(13, August, 2007);
    evaluationDate = UnitedKingdom().adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
                                         .withTenor(1*Months)
                                         .withCalendar(UnitedKingdom())
                                         .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
                       192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
                       194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
                       198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                       202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
                       207.3, -999.0, -999.0 };

    bool interp = true;
    boost::shared_ptr<YYUKRPIr> iir(new YYUKRPIr(Monthly, false, interp));
    for (Size i=0; i<rpiSchedule.size();i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
    }

    Date todayMinusLag = evaluationDate - iir->availabilityLag();
    std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, iir->frequency());
    todayMinusLag = lim.second + 1;

    Real eps = 1.0e-8;

    // -2 because last two values are not yet available,
    // (no TS so can't forecast).
    for (Size i=13; i<rpiSchedule.size()-2;i++) {
        std::pair<Date,Date> lim = inflationPeriod(rpiSchedule[i],
                                                   iir->frequency());
        for (Date d=lim.first; d<=lim.second; d++) {
            if (d <= todayMinusLag) {
                Rate expected = fixData[i]/fixData[i-12] - 1.0;
                Rate calculated = iir->fixing(d);
                if (std::fabs(calculated - expected) > eps)
                    BOOST_ERROR("Fixings not constant within a period: "
                                << calculated
                                << ", should be "
                                << expected);
            }
        }
    }
}


void InflationTest::testZeroIndex() {
    BOOST_MESSAGE("Testing zero inflation indices...");

    SavedSettings backup;

    EUHICP euhicp(Monthly, false, false);
    if (euhicp.name() != "EU HICP"
        || euhicp.frequency() != Monthly
        || euhicp.revised()
        || euhicp.interpolated()
        || euhicp.availabilityLag() != 3*Months) {
        BOOST_ERROR("wrong EU HICP data ("
                    << euhicp.name() << ", "
                    << euhicp.frequency() << ", "
                    << euhicp.revised() << ", "
                    << euhicp.interpolated() << ", "
                    << euhicp.availabilityLag() << ")");
    }

    UKRPI ukrpi(Monthly, true, true);
    if (ukrpi.name() != "UK RPI"
        || ukrpi.frequency() != Monthly
        || !ukrpi.revised()
        || !ukrpi.interpolated()
        || ukrpi.availabilityLag() != 2*Months) {
        BOOST_ERROR("wrong UK RPI data ("
                    << ukrpi.name() << ", "
                    << ukrpi.frequency() << ", "
                    << ukrpi.revised() << ", "
                    << ukrpi.interpolated() << ", "
                    << ukrpi.availabilityLag() << ")");
    }

    // Retrieval test.
    //----------------
    // make sure of the evaluation date
    Date evaluationDate(13, August, 2007);
    evaluationDate = UnitedKingdom().adjust(evaluationDate);
    Settings::instance().evaluationDate() = evaluationDate;

    // fixing data
    Date from(1, January, 2005);
    Date to(13, August, 2007);
    Schedule rpiSchedule = MakeSchedule().from(from).to(to)
                                         .withTenor(1*Months)
                                         .withCalendar(UnitedKingdom())
                                         .withConvention(ModifiedFollowing);

    Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
                       192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
                       194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
                       198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                       202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
                       207.3, -999.0, -999.0 };

    bool interp = true;
    boost::shared_ptr<UKRPI> iir(new UKRPI(Monthly, false, interp));
    for (Size i=0; i<rpiSchedule.size();i++) {
        iir->addFixing(rpiSchedule[i], fixData[i]);
    }

    Date todayMinusLag = evaluationDate - iir->availabilityLag();
    std::pair<Date,Date> lim = inflationPeriod(todayMinusLag, iir->frequency());
    todayMinusLag = lim.second + 1;

    Real eps = 1.0e-8;

    // -2 because last two values are not yet available,
    // (no TS so can't forecast).
    for (Size i=0; i<rpiSchedule.size()-2;i++) {
        std::pair<Date,Date> lim = inflationPeriod(rpiSchedule[i],
                                                   iir->frequency());
        for (Date d=lim.first; d<=lim.second; d++) {
            if (d <= todayMinusLag) {
                if (std::fabs(iir->fixing(d) - fixData[i]) > eps)
                    BOOST_ERROR("Fixings not constant within a period: "
                                << iir->fixing(d)
                                << ", should be " << fixData[i]);
            }
        }
    }
}


test_suite* InflationTest::suite() {

    test_suite* suite = BOOST_TEST_SUITE("Inflation tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testZeroIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testYYIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testZeroTermStructure));
    suite->add(QUANTLIB_TEST_CASE(&InflationTest::testYoYTermStructure));

    return suite;
}

