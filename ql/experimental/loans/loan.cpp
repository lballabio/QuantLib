#include <ql/experimental/loans/loan.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/settings.hpp>
#include <ql/cashflows/cashflows.hpp>

/*
    Just copy and paste the Bond class logic, is there a way to use derived class methods in parent constructors?
*/

namespace QuantLib {
    Loan::Loan(Natural settlementDays, Calendar calendar, const Date& issueDate, const Leg& coupons) : 
        Bond(settlementDays, std::move(calendar), issueDate) {
        if (!coupons.empty()) {
            std::sort(cashflows_.begin(), cashflows_.end(),
                      earlier_than<ext::shared_ptr<CashFlow> >());

            if (issueDate_ != Date()) {
                QL_REQUIRE(issueDate_ < cashflows_[0]->date(),
                           "issue date (" << issueDate_
                                          << ") must be earlier than first payment date ("
                                          << cashflows_[0]->date() << ")");
            }

            maturityDate_ = coupons.back()->date();
            //needs to be loan definition.
            addRedemptionsToCashflows();
        }

        registerWith(Settings::instance().evaluationDate());
        for (const auto& cashflow : cashflows_)
            registerWith(cashflow);
    }
    Loan::Loan(Natural settlementDays,
               Calendar calendar,
               Real faceAmount,
               const Date& maturityDate,
               const Date& issueDate,
               const Leg& cashflows)
    : Bond(settlementDays, calendar, faceAmount, maturityDate, issueDate, cashflows) {}
        
        
    void Loan::calculateNotionalsFromCashflows() {
        notionalSchedule_.clear();
        notionals_.clear();

        Date lastPaymentDate = Date();
        notionalSchedule_.emplace_back();
        for (auto& cashflow : cashflows_) {
            ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(cashflow);
            if (!coupon)
                continue;

            Real notional = coupon->nominal();
            // we add the notional only if it is the first one...
            if (notionals_.empty()) {
                notionals_.push_back(coupon->nominal());
                lastPaymentDate = coupon->date();
            } else if (!close(notional, notionals_.back())) {
                // ...or if it has changed.
                
                //QL_REQUIRE(notional < notionals_.back(), "increasing coupon notionals"); -> not required for loans!
                notionals_.push_back(coupon->nominal());
                // in this case, we also add the last valid date for
                // the previous one...
                notionalSchedule_.push_back(lastPaymentDate);
                // ...and store the candidate for this one.
                lastPaymentDate = coupon->date();
            } else {
                // otherwise, we just extend the valid range of dates
                // for the current notional.
                lastPaymentDate = coupon->date();
            }
        }
        QL_REQUIRE(!notionals_.empty(), "no coupons provided");
        notionals_.push_back(0.0);
        notionalSchedule_.push_back(lastPaymentDate);
    }
}
