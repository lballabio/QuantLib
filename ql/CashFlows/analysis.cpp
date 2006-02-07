/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/analysis.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace {

        // utility functions

        template <class T>
        Integer sign(T x) {
            static T zero = T();
            if (x == zero)
                return 0;
            else if (x > zero)
                return 1;
            else
                return -1;
        }

        class irrFinder : public std::unary_function<Rate,Real> {
          public:
            irrFinder(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   Real marketPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate)
            : cashflows_(cashflows), marketPrice_(marketPrice),
              dayCounter_(dayCounter), compounding_(compounding),
              frequency_(frequency), settlementDate_(settlementDate) {}
            Real operator()(Rate guess) const {
                InterestRate y(guess, dayCounter_, compounding_, frequency_);
                Real NPV = Cashflows::npv(cashflows_,y,settlementDate_);
                return marketPrice_ - NPV;
            }
          private:
            const std::vector<boost::shared_ptr<CashFlow> >& cashflows_;
            Real marketPrice_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            Date settlementDate_;
        };

        class BPSCalculator : public AcyclicVisitor,
                              public Visitor<CashFlow>,
                              public Visitor<Coupon> {
          public:
            BPSCalculator(const Handle<YieldTermStructure>& termStructure)
            : termStructure_(termStructure), result_(0.0) {}
            void visit(Coupon& c)  {
                result_ += c.accrualPeriod() *
                           c.nominal() *
                           termStructure_->discount(c.date());
            }
            void visit(CashFlow&) {}
            Real result() const { return result_; }
          private:
            Handle<YieldTermStructure> termStructure_;
            Real result_;
        };

    }


    Real Cashflows::npv(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const Handle<YieldTermStructure>& discountCurve) {
        const Date& settlementDate = discountCurve->referenceDate();
        Real totalNPV = 0.0;
        for (Size i = 0; i <cashflows.size(); i++) {
            if (!cashflows[i]->hasOccurred(settlementDate))
                totalNPV += cashflows[i]->amount() *
                            discountCurve->discount(cashflows[i]->date());
        }
        return totalNPV;
    }

    Real Cashflows::npv(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const InterestRate& irr,
                   Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();
        boost::shared_ptr<YieldTermStructure> flatRate(
                 new FlatForward(settlementDate, irr.rate(), irr.dayCounter(),
                                 irr.compounding(), irr.frequency()));
        return npv(cashflows, Handle<YieldTermStructure>(flatRate));
    }

    Real Cashflows::bps(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const Handle<YieldTermStructure>& discountCurve) {
        static const Spread basisPoint = 1.0e-4;
        const Date& settlementDate = discountCurve->referenceDate();
        BPSCalculator calc(discountCurve);
        for (Size i = 0; i <cashflows.size(); i++) {
            if (!cashflows[i]->hasOccurred(settlementDate))
                cashflows[i]->accept(calc);
        }
        return basisPoint*calc.result();
    }

    Real Cashflows::bps(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const InterestRate& irr,
                   Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();
        boost::shared_ptr<YieldTermStructure> flatRate(
                 new FlatForward(settlementDate, irr.rate(), irr.dayCounter(),
                                 irr.compounding(), irr.frequency()));
        return bps(cashflows, Handle<YieldTermStructure>(flatRate));
    }

    Rate Cashflows::irr(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   Real marketPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate,
                   Real tolerance,
                   Size maxIterations,
                   Rate guess) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        // depending on the sign of the market price, check that cash
        // flows of the opposite sign have been specified (otherwise
        // IRR is nonsensical.)

        Integer lastSign = sign(-marketPrice),
                signChanges = 0;
        for (Size i = 0; i < cashflows.size(); i++) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Integer thisSign = sign(cashflows[i]->amount());
                if (lastSign * thisSign < 0) // sign change
                    signChanges++;

                if (thisSign != 0)
                    lastSign = thisSign;
            }
        }
        QL_REQUIRE(signChanges > 0,
                  "the given cash flows cannot result in the given market "
                   "price due to their sign");

        /* The following is commented out due to the lack of a QL_WARN macro
        if (signChanges > 1) {    // Danger of non-unique solution
                                  // Check the aggregate cash flows (Norstrom)
            Real aggregateCashFlow = marketPrice;
            signChanges = 0;
            for (Size i = 0; i < cashflows.size(); i++) {
                Real nextAggregateCashFlow =
                    aggregateCashFlow + cashflows[i]->amount();

                if (aggregateCashFlow * nextAggregateCashFlow < 0.0)
                    signChanges++;

                aggregateCashFlow = nextAggregateCashFlow;
            }
            if (signChanges > 1)
                QL_WARN( "danger of non-unique solution");
        };
        */

        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        return solver.solve(irrFinder(cashflows, marketPrice, dayCounter,
                                      compounding, frequency, settlementDate),
                            tolerance, guess, guess/10.0);
    }

    Real Cashflows::convexity(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const InterestRate& r,
                   Date settlementDate) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        DayCounter dayCounter = r.dayCounter();

        Real totalConvexity = 0.0;
        for (Size i = 0; i < cashflows.size(); i++) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Time t = dayCounter.yearFraction(settlementDate,
                                                 cashflows[i]->date());
                DiscountFactor discount = r.discountFactor(t);
                totalConvexity += t * t * cashflows[i]->amount() * discount;
            }
        }
        return totalConvexity;
    }

    Time Cashflows::duration(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   Real marketPrice,
                   const InterestRate& rate,
                   Duration::Type type,
                   Date settlementDate) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Real totalDuration = 0.0;
        Real totalNPV = 0.0;

        Rate y = 0.0;
        if (type == Duration::Macaulay || type == Duration::Modified) {
            y = irr(cashflows, marketPrice, rate.dayCounter(),
                    rate.compounding(), rate.frequency(), settlementDate);
        }

        for (Size i = 0; i < cashflows.size(); i++) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Time t = rate.dayCounter().yearFraction(settlementDate,
                                                        cashflows[i]->date());
                Real c = cashflows[i]->amount();
                DiscountFactor discount;
                if (type == Duration::Macaulay)
                    discount = std::exp(-y*t);
                else
                    discount = rate.discountFactor(t);

                totalNPV += c * discount;
                totalDuration += t * c * discount;
            }
        }
        totalNPV -= marketPrice;

        if (totalNPV == 0.0)
            // what to do?
            return 0.0;

        switch (type){
          case Duration::Modified:
            return totalDuration / totalNPV / y;
          case Duration::Simple:
          case Duration::Macaulay:
            return totalDuration/totalNPV;
          default:
            QL_FAIL("unknown duration type");
        }
    }

}
