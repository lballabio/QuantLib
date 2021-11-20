#pragma once

#include <ql/instruments/bond.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/cashflow.hpp>
#include <ql/cashflows/simplecashflow.hpp>

namespace QuantLib {
    class Loan : public Bond {
      public:
        Loan(Natural settlementDays,
             Calendar calendar,             
             Real initialPayment,
             const Date& issueDate,
             const Leg& coupons = Leg());

        Loan(Natural settlementDays,
             Calendar calendar,
             Real faceAmount,
             const Date& maturityDate,
             const Date& issueDate,
             const Leg& cashflows = Leg());
        Redemption& initialPayment();
     protected:   
        bool validateRedemptions();
     private:
        void calculateNotionalsFromCashflows() override;
        Redemption initialPayment_;
    };
}