#pragma once
#include <ql/experimental/loans/loan.hpp>
#include <ql/time/daycounter.hpp>

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

    };
}