#pragma once

#include <ql/quotes/simplequote.hpp>
#include <ql/qldefines.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>
#include <ql/time/date.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/experimental/loans/amortizingloans.hpp>
#include <ql/cashflow.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

#include <iostream>


using namespace QuantLib;

void printLoanCashflows(Loan& loan) {
    std::cout << "Loan cashflows:" << std::endl;
    for (auto& cf : loan.cashflows()) {
        std::cout << cf->date() << " " << cf->amount() << std::endl;
    }
}
/*
        Loan example:
            - Instatiate class as normal.
            - Check increasing notionals.
            - Try to price it.
*/
void loanExample() {
    Natural settlementDays = 0;
    Date startDate(12, Nov, 2021);
    Date endDate(12, Nov, 2023);
    Frequency freq = Semiannual;
    
    Settings::instance().evaluationDate() = startDate;
   
    Compounding comp = Simple;
    Calendar calendar = NullCalendar();
    Rate coupon = 0.06;
    Rate curveRate = 0.03;
    BusinessDayConvention accrualConvention = Unadjusted;
    BusinessDayConvention paymentConvention = Unadjusted;
    DayCounter accrualDayCounter = Thirty360(Thirty360::BondBasis);
    Schedule loanSchedule(startDate, endDate, Period(freq), calendar, accrualConvention,
                          accrualConvention, DateGeneration::Backward, false);

    //Notionals increase for 2 coupons and then decrease for 2 coupons
    std::vector<Real> notionals;
    Real increment = 10;
    Real intialPayment = 10;
    
    Redemption cf(intialPayment, startDate);

    notionals.push_back(increment);
    for (size_t i = 1; i < loanSchedule.size()-1; i++) {
        if (i <= loanSchedule.size()/2) {
            notionals.push_back(notionals[i - 1] + increment);
        } 
        else {
            notionals.push_back(notionals[i - 1] - increment);
        }
    }
    Leg loanLeg = FixedRateLeg (loanSchedule).withNotionals(notionals)
                    .withCouponRates(coupon, accrualDayCounter)
                    .withFirstPeriodDayCounter(accrualDayCounter)
                    .withPaymentCalendar(calendar)
                    .withPaymentAdjustment(paymentConvention)
                    .withExCouponPeriod(Period(), Calendar(), Unadjusted, false);


    Loan loan(settlementDays, calendar, intialPayment, startDate, loanLeg);    
    std::cout << "Increasing notional loan cashflows:" << std::endl;
    printLoanCashflows(loan);

    /*
        Loan pricing -> same as a bond pricing 
        todo: add parRate solver
    */    
    ext::shared_ptr<Quote> yield(new SimpleQuote(curveRate));
    ext::shared_ptr<YieldTermStructure> discountingCurve(
        new FlatForward(startDate, Handle<Quote>(yield), accrualDayCounter));
    Handle<YieldTermStructure> discountingCurveHandle(discountingCurve);
    ext::shared_ptr<PricingEngine> loanEngine(new DiscountingBondEngine(discountingCurveHandle, true));
    
    loan.setPricingEngine(loanEngine);

    std::cout << "Loan NPV:" << loan.NPV() << std::endl;
    std::cout << std::endl;
    Real y = CashFlows::parRate(loan.cashflows(), loan.initialPayment(), *discountingCurve);
    std::cout << "Loan par rate:" << y << std::endl;
    std::cout << std::endl;
    
}
/*
        EqualPaymentLoan example.                  
*/
void equalPaymentLoanExample() {
    Natural settlementDays = 0;
    Date startDate(12, Nov, 2021);
    Date endDate(12, Nov, 2023);
    Frequency paymentFreq = Semiannual;
    Frequency couponFreq = Annual;

    Settings::instance().evaluationDate() = startDate;

    Compounding comp = Simple;
    Calendar calendar = NullCalendar();
    Rate coupon = 0.06;
    Rate curveRate = 0.03;
    Real faceAmount = 1000;
    BusinessDayConvention accrualConvention = Unadjusted;
    BusinessDayConvention paymentConvention = Unadjusted;
    DayCounter accrualDayCounter = Thirty360(Thirty360::BondBasis);
    Schedule loanSchedule(startDate, endDate, Period(paymentFreq), calendar, accrualConvention,
                          accrualConvention, DateGeneration::Backward, false);


    EqualPaymentLoan loan(settlementDays, faceAmount, loanSchedule, coupon, couponFreq, comp,
                          accrualDayCounter);

    std::cout << "Equal payment loan cashflows:" << std::endl;
    printLoanCashflows(loan);    
    /*
        Loan pricing -> same as a bond pricing
    */
    ext::shared_ptr<Quote> yield(new SimpleQuote(curveRate));
    ext::shared_ptr<YieldTermStructure> discountingCurve(
        new FlatForward(startDate, Handle<Quote>(yield), accrualDayCounter));
    Handle<YieldTermStructure> discountingCurveHandle(discountingCurve);
    ext::shared_ptr<PricingEngine> loanEngine(
        new DiscountingBondEngine(discountingCurveHandle, true));
    loan.setPricingEngine(loanEngine);

    std::cout << "Loan NPV:" << loan.NPV() << std::endl;
    std::cout << std::endl;

    std::cout << "Loan par rate:" << CashFlows::parRate(loan.cashflows(), loan.initialPayment(), *discountingCurve) * 100 << std::endl;
    std::cout << std::endl;
}
/*
        EqualRedeptionLoan example.         
*/
void equalRedemptionLoanExample() {
    Natural settlementDays = 0;
    Date startDate(12, Nov, 2021);
    Date endDate(12, Nov, 2023);
    Frequency paymentFreq = Semiannual;
    Frequency couponFreq = Annual;

    Settings::instance().evaluationDate() = startDate;

    Compounding comp = Simple;
    Calendar calendar = NullCalendar();
    Rate coupon = 0.06;
    Rate curveRate = 0.03;
    Real faceAmount = 1000;
    BusinessDayConvention accrualConvention = Unadjusted;
    BusinessDayConvention paymentConvention = Unadjusted;
    DayCounter accrualDayCounter = Thirty360(Thirty360::BondBasis);
    Schedule loanSchedule(startDate, endDate, Period(paymentFreq), calendar, accrualConvention,
                          accrualConvention, DateGeneration::Backward, false);

    
    EqualRedemptionFixedRateLoan loan(settlementDays, faceAmount, loanSchedule, coupon, accrualDayCounter);

    std::cout << "Equal redemption loan cashflows:" << std::endl;
    printLoanCashflows(loan);
 

    /*
        Loan pricing -> same as a bond pricing
    */
    ext::shared_ptr<Quote> yield(new SimpleQuote(curveRate));
    ext::shared_ptr<YieldTermStructure> discountingCurve(
        new FlatForward(startDate, Handle<Quote>(yield), accrualDayCounter));
    Handle<YieldTermStructure> discountingCurveHandle(discountingCurve);
    ext::shared_ptr<PricingEngine> loanEngine(
        new DiscountingBondEngine(discountingCurveHandle, true));
    loan.setPricingEngine(loanEngine);

    std::cout << "Loan NPV:" << loan.NPV() << std::endl;
    std::cout << std::endl;
    std::cout << "Loan par rate:"
              << CashFlows::parRate(loan.cashflows(), loan.initialPayment(), *discountingCurve) *
                     100
              << std::endl;
    std::cout << std::endl;
}

int main() {
    
    loanExample();
    equalPaymentLoanExample();
    equalRedemptionLoanExample();

}


