#pragma once

#include <ql/experimental/loans/amortizingloans.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/time/schedule.hpp>

#include <ql/math/matrix.hpp>
#include <ql/math/array.hpp>
#include <iostream>

namespace QuantLib {
    
    EqualPaymentLoan::EqualPaymentLoan(Natural settlementDays,
                                       Real faceAmount,
                                       const Schedule& schedule,
                                       const Rate& coupon,
                                       const Frequency& freq,
                                       const Compounding& comp,
                                       const DayCounter& accrualDayCounter,
                                       BusinessDayConvention paymentConvention,
                                       const Date& issueDate,
                                       const Calendar& paymentCalendar,
                                       const Period& exCouponPeriod,
                                       const Calendar& exCouponCalendar,
                                       BusinessDayConvention exCouponConvention,
                                       bool exCouponEndOfMonth,
                                       const DayCounter& firstPeriodDayCounter)
    : Loan(settlementDays,
           paymentCalendar == Calendar() ? schedule.calendar() : paymentCalendar, faceAmount,
           issueDate),
      frequency_(schedule.hasTenor() ? schedule.tenor().frequency() : NoFrequency),
      dayCounter_(accrualDayCounter), firstPeriodDayCounter_(firstPeriodDayCounter) {

        // Calculate redemtion schedule, solved as a system of equations
        // equations: 3 payments + 1 faceamount=sum(redemtions)      
        InterestRate r(coupon, accrualDayCounter, comp, freq);
        std::vector<Real> notionals;
        calculateNotionals(r, faceAmount, schedule, notionals);
        
        //same as fixedratebond
        maturityDate_ = schedule.endDate();
        cashflows_ = FixedRateLeg(schedule)
                         .withNotionals(notionals)
                         .withCouponRates(coupon, accrualDayCounter)
                         .withFirstPeriodDayCounter(firstPeriodDayCounter)
                         .withPaymentCalendar(calendar_)
                         .withPaymentAdjustment(paymentConvention)
                         .withExCouponPeriod(exCouponPeriod, exCouponCalendar, exCouponConvention,
                                             exCouponEndOfMonth);

        addRedemptionsToCashflows();
        QL_ENSURE(validateRedemptions(), "redemptions must sum 0");
        QL_ENSURE(!cashflows().empty(), "loan with no cashflows!");
    }

    void EqualPaymentLoan::calculateNotionals(InterestRate& rate, Real faceAmount, const Schedule& schedule, std::vector<Real>& notionals) 
    {
        size_t paymentNum = schedule.size() - 1;
        size_t eqNum = paymentNum + 1;
        Array factors(eqNum, 0), B(eqNum, 0), K;

        for (size_t i = 1; i <= paymentNum; i++) {
            factors[i - 1] = (rate.compoundFactor(schedule[i - 1], schedule[i]) - 1);
        }
        factors[eqNum - 1] = -1;
        B = -faceAmount * factors;
        Matrix A(eqNum, eqNum, 0);
        for (size_t j = 0; j < eqNum; j++) {
            for (size_t i = 0; i < eqNum; i++) {
                if (j == 0 && i < eqNum - 1) {
                    A[i][j] = -1;
                } else if (j > 0 && i == j - 1) {
                    A[i][j] = 1;
                } else if (j > 0 && j <= i && i < eqNum - 1) {
                    A[i][j] = -factors[i];
                } else if (i == eqNum - 1 && j > 0) {
                    A[i][j] = 1;
                }
            }
        }

        K = inverse(A) * B;   
        notionals.clear();
        notionals.push_back(faceAmount);
        //std::cout << K << std::endl;
        for (size_t i = 0; i < paymentNum; i++) {
            notionals.push_back(notionals[i] - K[i + 1]);            
        }  
    }

    EqualRedemptionFixedRateLoan::EqualRedemptionFixedRateLoan(
        Natural settlementDays,
        Real faceAmount,
        const Schedule& schedule,
        const Rate& coupon,
        const DayCounter& accrualDayCounter,
        BusinessDayConvention paymentConvention,
        const Date& issueDate,
        const Calendar& paymentCalendar,
        const Period& exCouponPeriod,
        const Calendar& exCouponCalendar,
        BusinessDayConvention exCouponConvention,
        bool exCouponEndOfMonth,
        const DayCounter& firstPeriodDayCounter)
    : Loan(settlementDays,
           paymentCalendar == Calendar() ? schedule.calendar() : paymentCalendar, faceAmount,
           issueDate),
      frequency_(schedule.hasTenor() ? schedule.tenor().frequency() : NoFrequency),
      dayCounter_(accrualDayCounter), firstPeriodDayCounter_(firstPeriodDayCounter) {
        Real size = schedule.size();
        Real redemption = faceAmount / (size - 1.0);
        std::vector<Real> notionals(size, 0);
        notionals[0] = faceAmount;
        for (size_t i = 1; i < size; i++) {
            notionals[i] = notionals[i - 1] - redemption;
        }
        maturityDate_ = schedule.endDate();
        cashflows_ = FixedRateLeg(schedule)
                         .withNotionals(notionals)
                         .withCouponRates(coupon, accrualDayCounter)
                         .withFirstPeriodDayCounter(firstPeriodDayCounter)
                         .withPaymentCalendar(calendar_)
                         .withPaymentAdjustment(paymentConvention)
                         .withExCouponPeriod(exCouponPeriod, exCouponCalendar, exCouponConvention,
                                             exCouponEndOfMonth);

        addRedemptionsToCashflows();
        QL_ENSURE(validateRedemptions(), "redemptions must sum 0");
        QL_ENSURE(!cashflows().empty(), "loan with no cashflows!");
    }

    BulletFixedRateLoan::BulletFixedRateLoan(Natural settlementDays,
                                             Real faceAmount,
                                             const Schedule& schedule,
                                             const Rate& coupons,
                                             const DayCounter& accrualDayCounter,
                                             BusinessDayConvention paymentConvention,
                                             Real redemption,
                                             const Date& issueDate,
                                             const Calendar& paymentCalendar,
                                             const Period& exCouponPeriod,
                                             const Calendar& exCouponCalendar,
                                             BusinessDayConvention exCouponConvention,
                                             bool exCouponEndOfMonth,
                                             const DayCounter& firstPeriodDayCounter)
    : Loan(settlementDays,
           paymentCalendar == Calendar() ? schedule.calendar() : paymentCalendar, faceAmount,
           issueDate),
      frequency_(schedule.hasTenor() ? schedule.tenor().frequency() : NoFrequency),
      dayCounter_(accrualDayCounter), firstPeriodDayCounter_(firstPeriodDayCounter) {
        
        maturityDate_ = schedule.endDate();
        cashflows_ = FixedRateLeg(schedule)
                         .withNotionals(faceAmount)
                         .withCouponRates(coupons, accrualDayCounter)
                         .withFirstPeriodDayCounter(firstPeriodDayCounter)
                         .withPaymentCalendar(calendar_)
                         .withPaymentAdjustment(paymentConvention)
                         .withExCouponPeriod(exCouponPeriod, exCouponCalendar, exCouponConvention,
                                             exCouponEndOfMonth);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));
        QL_ENSURE(validateRedemptions(), "redemptions must sum 0");
        QL_ENSURE(!cashflows().empty(), "loan with no cashflows!");
    }
    
}