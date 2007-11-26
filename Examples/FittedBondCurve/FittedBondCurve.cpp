/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2007 Allen Kuo

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

/*  This example shows how to fit a term structure to a set of bonds
    using four different fitting methodologies. Though fitting is most
    useful for large numbers of bonds with non-smooth yield tenor
    structures, for comparison purposes, relatively smooth bond yields
    are fit here and compared to known solutions (par coupons), or
    results generated from the bootstrap fitting method.
*/

#define BOOST_LIB_DIAGNOSTIC
#  include <ql/quantlib.hpp>
#undef BOOST_LIB_DIAGNOSTIC

#ifdef BOOST_MSVC
/* Uncomment the following lines to unmask floating-point
   exceptions. Warning: unpredictable results can arise...

   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
   Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {
    Integer sessionId() { return 0; }
}
#endif

void printOutput(const std::string& tag,
                 const boost::shared_ptr<FittedBondDiscountCurve>& curve) {
    cout << tag << endl;
    cout << "reference date : "
         << curve->referenceDate()
         << endl;
    cout << "number of iterations : "
         << curve->fitResults().numberOfIterations()
         << endl
         << endl;
}


int main(int, char* []) {

    try {

        boost::timer timer;

        const Size numberOfBonds = 15;
        Real cleanPrice[numberOfBonds];

        for (Size i=0; i<=numberOfBonds; i++) {
            cleanPrice[i]=100.0;
        }

        std::vector< boost::shared_ptr<SimpleQuote> > quote;
        for (Size i=0; i<numberOfBonds; i++) {
            boost::shared_ptr<SimpleQuote> cp(new SimpleQuote(cleanPrice[i]));
            quote.push_back(cp);
        }

        RelinkableHandle<Quote> quoteHandle[numberOfBonds];
        for (Size i=0; i<numberOfBonds; i++) {
            quoteHandle[i].linkTo(quote[i]);
        }

        Calendar calendar = NullCalendar();
        Date today = calendar.adjust(Date::todaysDate());
        Date origToday = calendar.adjust(Date::todaysDate());
        Settings::instance().evaluationDate() = today;

        cout << endl;
        cout << "Today's date: "
             << origToday
             << endl;
        cout << "Calculating fit for 15 bonds....."
             << endl
             << endl;

        Integer lengths[] = { 2, 4, 6, 8, 10, 12, 14, 16,
                              18, 20, 22, 24, 26, 28, 30 };
        Real coupons[] = { 0.0200, 0.0225, 0.0250, 0.0275, 0.0300,
                           0.0325, 0.0350, 0.0375, 0.0400, 0.0425,
                           0.0450, 0.0475, 0.0500, 0.0525, 0.0550 };

        Frequency frequency = Annual;
        DayCounter bondDayCount = SimpleDayCounter();
        BusinessDayConvention accrualConvention = Unadjusted;
        BusinessDayConvention convention = ModifiedFollowing;
        Real redemption = 100.0;

        // changing settlementDays=3 increases calculation time of
        // exponentialsplines fitting method considerably
        Natural settlementDays = 0;
        Natural curveSettlementDays = 0;

        std::vector<boost::shared_ptr<FixedRateBondHelper> > instrumentsA;
        std::vector<boost::shared_ptr<RateHelper> > instrumentsB;

        for (Size j=0; j<LENGTH(lengths); j++) {

            Date dated = origToday;
            Date issue = origToday;
            Date maturity = calendar.advance(issue, lengths[j], Years);

            Schedule schedule(dated, maturity, Period(frequency), calendar,
                              accrualConvention, accrualConvention,
                              DateGeneration::Backward, false);

            boost::shared_ptr<FixedRateBondHelper> helperA(
                   new FixedRateBondHelper(quoteHandle[j],
                                             settlementDays,
                                             schedule,
                                             std::vector<Rate>(1,coupons[j]),
                                             bondDayCount,
                                             convention,
                                             redemption,
                                             issue));

            boost::shared_ptr<RateHelper> helperB(
                   new FixedRateBondHelper(quoteHandle[j],
                                             settlementDays,
                                             schedule,
                                             std::vector<Rate>(1, coupons[j]),
                                             bondDayCount,
                                             convention,
                                             redemption,
                                             issue));
            instrumentsA.push_back(helperA);
            instrumentsB.push_back(helperB);
        }


        bool constrainAtZero = true;
        Real tolerance = 1.0e-10;
        Size max = 5000;

        boost::shared_ptr<YieldTermStructure> ts0 (
              new PiecewiseYieldCurve<Discount,LogLinear>(curveSettlementDays,
                                                          calendar,
                                                          instrumentsB,
                                                          bondDayCount));


        ExponentialSplinesFitting exponentialSplines(constrainAtZero);

        boost::shared_ptr<FittedBondDiscountCurve> ts1 (
                  new FittedBondDiscountCurve(curveSettlementDays,
                                              calendar,
                                              instrumentsA,
                                              bondDayCount,
                                              exponentialSplines,
                                              tolerance,
                                              max));

        printOutput("(a) exponential splines", ts1);


        SimplePolynomialFitting simplePolynomial(3, constrainAtZero);

        boost::shared_ptr<FittedBondDiscountCurve> ts2 (
                    new FittedBondDiscountCurve(curveSettlementDays,
                                                calendar,
                                                instrumentsA,
                                                bondDayCount,
                                                simplePolynomial,
                                                tolerance,
                                                max));

        printOutput("(b) simple polynomial", ts2);


        NelsonSiegelFitting nelsonSiegel;

        boost::shared_ptr<FittedBondDiscountCurve> ts3 (
                        new FittedBondDiscountCurve(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    bondDayCount,
                                                    nelsonSiegel,
                                                    tolerance,
                                                    max));

        printOutput("(c) Nelson Siegel", ts3);


        // a cubic bspline curve with 11 knot points, implies
        // n=6 (constrained problem) basis functions

        Time knots[] =  { -30.0, -20.0,  0.0,  5.0, 10.0, 15.0,
                           20.0,  25.0, 30.0, 40.0, 50.0 };

        std::vector<Time> knotVector;
        for (Size i=0; i< LENGTH(knots); i++) {
            knotVector.push_back(knots[i]);
        }

        CubicBSplinesFitting cubicBSplines(knotVector, constrainAtZero);

        boost::shared_ptr<FittedBondDiscountCurve> ts4 (
                       new FittedBondDiscountCurve(curveSettlementDays,
                                                   calendar,
                                                   instrumentsA,
                                                   bondDayCount,
                                                   cubicBSplines,
                                                   tolerance,
                                                   max));

        printOutput("(d) cubic B-splines", ts4);


        cout << "Output par rates for each curve. In this case, "
             << endl
             << "par rates should equal coupons for these par bonds."
             << endl
             << endl;

        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<boost::shared_ptr<CashFlow> > cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(today);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(today)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor=bondDayCount.yearFraction(today,cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts0->parRate(keyDates,Annual,false) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts1->parRate(keyDates,Annual,false) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts2->parRate(keyDates,Annual,false) << " | "
                 // nelson siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts3->parRate(keyDates,Annual,false) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts4->parRate(keyDates,Annual,false) << endl;
        }

        cout << endl << endl << endl;
        cout << "Now add 23 months to today. Par rates should be "  << endl
             << "automatically recalculated because today's date "  << endl
             << "changes.  Par rates will NOT equal coupons (YTM "  << endl
             << "will, with the correct compounding), but the "     << endl
             << "piecewise yield curve par rates can be used as "   << endl
             << "a benchmark for correct par rates."
             << endl
             << endl;

        today = calendar.advance(today,23,Months,convention);
        Settings::instance().evaluationDate() = today;

        printOutput("(a) exponential splines", ts1);

        printOutput("(b) simple polynomial", ts2);

        printOutput("(c) Nelson Siegel", ts3);

        printOutput("(d) cubic B-splines", ts4);

        cout << endl
             << endl;


        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<boost::shared_ptr<CashFlow> > cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(today);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(today)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor =
                bondDayCount.yearFraction(today,cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts0->parRate(keyDates,Annual,false) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts1->parRate(keyDates,Annual,false) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts2->parRate(keyDates,Annual,false) << " | "
                 // nelson siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts3->parRate(keyDates,Annual,false) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts4->parRate(keyDates,Annual,false) << endl;
        }

        cout << endl << endl << endl;
        cout << "Now add one more month, for a total of two years " << endl
             << "from the original date. The first instrument is "  << endl
             << "now expired and par rates should again equal "     << endl
             << "coupon values, since clean prices did not change."
             << endl
             << endl;

        instrumentsA.erase(instrumentsA.begin(),
                           instrumentsA.begin()+1);
        instrumentsB.erase(instrumentsB.begin(),
                           instrumentsB.begin()+1);

        today = calendar.advance(today,1,Months,convention);
        Settings::instance().evaluationDate() = today;

        boost::shared_ptr<YieldTermStructure> ts00 (
              new PiecewiseYieldCurve<Discount,LogLinear>(curveSettlementDays,
                                                          calendar,
                                                          instrumentsB,
                                                          bondDayCount));

        boost::shared_ptr<FittedBondDiscountCurve> ts11 (
                  new FittedBondDiscountCurve(curveSettlementDays,
                                              calendar,
                                              instrumentsA,
                                              bondDayCount,
                                              exponentialSplines,
                                              tolerance,
                                              max));

        printOutput("(a) exponential splines", ts11);


        boost::shared_ptr<FittedBondDiscountCurve> ts22 (
                    new FittedBondDiscountCurve(curveSettlementDays,
                                                calendar,
                                                instrumentsA,
                                                bondDayCount,
                                                simplePolynomial,
                                                tolerance,
                                                max));

        printOutput("(b) simple polynomial", ts22);


        boost::shared_ptr<FittedBondDiscountCurve> ts33 (
                        new FittedBondDiscountCurve(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    bondDayCount,
                                                    nelsonSiegel,
                                                    tolerance,
                                                    max));

        printOutput("(c) Nelson Siegel", ts33);


        boost::shared_ptr<FittedBondDiscountCurve> ts44 (
                       new FittedBondDiscountCurve(curveSettlementDays,
                                                   calendar,
                                                   instrumentsA,
                                                   bondDayCount,
                                                   cubicBSplines,
                                                   tolerance,
                                                   max));

        printOutput("(d) cubic B-splines", ts44);


        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<boost::shared_ptr<CashFlow> > cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(today);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(today)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor =
                bondDayCount.yearFraction(today,cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i+1] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts00->parRate(keyDates,Annual,false) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts11->parRate(keyDates,Annual,false) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts22->parRate(keyDates,Annual,false) << " | "
                 // nelson siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts33->parRate(keyDates,Annual,false) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts44->parRate(keyDates,Annual,false) << endl;
        }


        cout << endl << endl << endl;
        cout << "Now decrease prices by a small amount, corresponding"  << endl
             << "to a theoretical five basis point parallel + shift of" << endl
             << "the yield curve. Because bond quotes change, the new " << endl
             << "par rates should be recalculated automatically."
             << endl
             << endl;

        for (Size k=0; k<LENGTH(lengths); k++) {

            std::vector<boost::shared_ptr<CashFlow> > leg =
                instrumentsA[k]->bond()->cashflows();

            Real quotePrice = instrumentsA[k]->quoteValue();
            Rate ytm =
                instrumentsA[k]->bond()->yield(quotePrice,
                                               instrumentsA[k]->dayCounter(),
                                               Compounded,
                                               instrumentsA[k]->frequency(),
                                               today);
            InterestRate r(ytm,
                           instrumentsA[k]->dayCounter(),
                           Compounded,
                           instrumentsA[k]->frequency());
            Time dur = CashFlows::duration(leg, r, Duration::Modified, today);

            const Real BpsChange = 5.;
            // dP = -dur*P * dY
            Real deltaP = - dur*cleanPrice[k]*(BpsChange/10000.);
            quote[k]->setValue(cleanPrice[k] + deltaP);
        }


        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<boost::shared_ptr<CashFlow> > cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(today);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(today)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor =
                bondDayCount.yearFraction(today,cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i+1] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts00->parRate(keyDates,Annual,false) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts11->parRate(keyDates,Annual,false) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts22->parRate(keyDates,Annual,false) << " | "
                 // nelson siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts33->parRate(keyDates,Annual,false) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*ts44->parRate(keyDates,Annual,false) << endl;
        }


        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nRun completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;

        return 0;

    } catch (std::exception& e) {
        cout << e.what() << endl;
        return 1;
    } catch (...) {
        cout << "unknown error" << endl;
        return 1;
    }

}






