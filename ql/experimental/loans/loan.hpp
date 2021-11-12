#include <ql/instruments/bond.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/cashflow.hpp>



namespace QuantLib {
    class Loan : public Bond {
      public:
        Loan(Natural settlementDays,
             Calendar calendar,
             const Date& issueDate,
             const Leg& coupons = Leg());

        Loan(Natural settlementDays,
             Calendar calendar,
             Real faceAmount,
             const Date& maturityDate,
             const Date& issueDate,
             const Leg& cashflows = Leg());
        void calculateNotionalsFromCashflows() override;
    };

}