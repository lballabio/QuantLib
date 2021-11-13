#pragma once
#include <ql/experimental/loans/loan.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/interestrate.hpp>

namespace QuantLib {
    class Schedule;

    class EqualPaymentLoan : public Loan {
      public:
        EqualPaymentLoan(Natural settlementDays,
                         Real faceAmount,
                         const Schedule& schedule,
                         const Rate& coupon,
                         const Frequency& freq,
                         const Compounding& comp,
                         const DayCounter& accrualDayCounter,
                         BusinessDayConvention paymentConvention = Following,                         
                         const Date& issueDate = Date(),
                         const Calendar& paymentCalendar = Calendar(),
                         const Period& exCouponPeriod = Period(),
                         const Calendar& exCouponCalendar = Calendar(),
                         BusinessDayConvention exCouponConvention = Unadjusted,
                         bool exCouponEndOfMonth = false,
                         const DayCounter& firstPeriodDayCounter = DayCounter());
        
        Frequency frequency() const { return frequency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        const DayCounter& firstPeriodDayCounter() const { return firstPeriodDayCounter_; }
        
      protected:        
        Frequency frequency_;
        DayCounter dayCounter_;
        DayCounter firstPeriodDayCounter_;

      private:
        void calculateNotionals(InterestRate& rate, Real faceAmount, const Schedule& schedule, std::vector<Real>& notionals);

    };
    class EqualRedemptionFixedRateLoan : public Loan {
      public:
        EqualRedemptionFixedRateLoan(Natural settlementDays,
                                     Real faceAmount,
                                     const Schedule& schedule,
                                     const Rate& coupon,                                     
                                     const DayCounter& accrualDayCounter,
                                     BusinessDayConvention paymentConvention = Following,
                                     const Date& issueDate = Date(),
                                     const Calendar& paymentCalendar = Calendar(),
                                     const Period& exCouponPeriod = Period(),
                                     const Calendar& exCouponCalendar = Calendar(),
                                     BusinessDayConvention exCouponConvention = Unadjusted,
                                     bool exCouponEndOfMonth = false,
                                     const DayCounter& firstPeriodDayCounter = DayCounter());

        Frequency frequency() const { return frequency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        const DayCounter& firstPeriodDayCounter() const { return firstPeriodDayCounter_; }

      protected:
        Frequency frequency_;
        DayCounter dayCounter_;
        DayCounter firstPeriodDayCounter_;
    };
    class BulletFixedRateLoan : public Loan {
      public:
        BulletFixedRateLoan(Natural settlementDays,
                                     Real faceAmount,
                                     const Schedule& schedule,
                                     const Rate& coupon,
                                     const DayCounter& accrualDayCounter,
                                     BusinessDayConvention paymentConvention = Following,
                                     Real redemption = 100,
                                     const Date& issueDate = Date(),
                                     const Calendar& paymentCalendar = Calendar(),
                                     const Period& exCouponPeriod = Period(),
                                     const Calendar& exCouponCalendar = Calendar(),
                                     BusinessDayConvention exCouponConvention = Unadjusted,
                                     bool exCouponEndOfMonth = false,
                                     const DayCounter& firstPeriodDayCounter = DayCounter());

        Frequency frequency() const { return frequency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        const DayCounter& firstPeriodDayCounter() const { return firstPeriodDayCounter_; }

      protected:
        Frequency frequency_;
        DayCounter dayCounter_;
        DayCounter firstPeriodDayCounter_;
    };
}