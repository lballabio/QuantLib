/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009, 2010, 2011 Ferdinando Ametrano
 Copyright (C) 2008 Toyin Akin

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

/*! \file cashflows.hpp
    \brief Cash-flow analysis functions
*/

#ifndef quantlib_cashflows_hpp
#define quantlib_cashflows_hpp

#include <ql/cashflows/duration.hpp>
#include <ql/cashflow.hpp>
#include <ql/interestrate.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    class YieldTermStructure;

    //! %cashflow-analysis functions
    /*! \todo add tests */
    class CashFlows {
      private:
        class IrrFinder {
          public:
            IrrFinder(const Leg& leg,
                      Real npv,
                      DayCounter dayCounter,
                      Compounding comp,
                      Frequency freq,
                      bool includeSettlementDateFlows,
                      Date settlementDate,
                      Date npvDate);

            Real operator()(Rate y) const;
            Real derivative(Rate y) const;
          private:
            void checkSign() const;

            const Leg& leg_;
            Real npv_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            bool includeSettlementDateFlows_;
            Date settlementDate_, npvDate_;
        };
      public:
        CashFlows() = delete;
        CashFlows(CashFlows&&) = delete;
        CashFlows(const CashFlows&) = delete;
        CashFlows& operator=(CashFlows&&) = delete;
        CashFlows& operator=(const CashFlows&) = delete;
        ~CashFlows() = default;

        //! \name Date functions
        //@{
        static Date startDate(const Leg& leg);
        static Date maturityDate(const Leg& leg);
        static bool isExpired(const Leg& leg,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date());
        //@}

        //! \name CashFlow functions
        //@{
        //! the last cashflow paying before or at the given date
        static Leg::const_reverse_iterator
        previousCashFlow(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlementDate = Date());
        //! the first cashflow paying after the given date
        static Leg::const_iterator
        nextCashFlow(const Leg& leg,
                     bool includeSettlementDateFlows,
                     Date settlementDate = Date());
        static Date
        previousCashFlowDate(const Leg& leg,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date());
        static Date
        nextCashFlowDate(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlementDate = Date());
        static Real
        previousCashFlowAmount(const Leg& leg,
                               bool includeSettlementDateFlows,
                               Date settlementDate = Date());
        static Real
        nextCashFlowAmount(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlementDate = Date());
        //@}

        //! \name Coupon inspectors
        //@{
        static Rate
        previousCouponRate(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlementDate = Date());
        static Rate
        nextCouponRate(const Leg& leg,
                       bool includeSettlementDateFlows,
                       Date settlementDate = Date());

        static Real
        nominal(const Leg& leg,
                bool includeSettlementDateFlows,
                Date settlDate = Date());
        static Date
        accrualStartDate(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlDate = Date());
        static Date
        accrualEndDate(const Leg& leg,
                       bool includeSettlementDateFlows,
                       Date settlementDate = Date());
        static Date
        referencePeriodStart(const Leg& leg,
                             bool includeSettlementDateFlows,
                             Date settlDate = Date());
        static Date
        referencePeriodEnd(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlDate = Date());
        static Time
        accrualPeriod(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        static Date::serial_type
        accrualDays(const Leg& leg,
                    bool includeSettlementDateFlows,
                    Date settlementDate = Date());
        static Time
        accruedPeriod(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        static Date::serial_type
        accruedDays(const Leg& leg,
                    bool includeSettlementDateFlows,
                    Date settlementDate = Date());
        static Real
        accruedAmount(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        //@}

        //! \name YieldTermStructure functions
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given term structure.
        */
        static Real npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given term structure.
        */
        static Real bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());

        //! NPV and BPS of the cash flows.
        /*! The NPV and BPS of the cash flows calculated
            together for performance reason
        */
        static std::pair<Real, Real> npvbps(const Leg& leg,
                                            const YieldTermStructure& discountCurve,
                                            bool includeSettlementDateFlows,
                                            Date settlementDate = Date(),
                                            Date npvDate = Date());

        //! NPV and BPS of the cash flows.
        /*! \deprecated Use the overload returning a pair of Reals.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        static void npvbps(const Leg& leg,
                           const YieldTermStructure& discountCurve,
                           bool includeSettlementDateFlows,
                           Date settlementDate,
                           Date npvDate,
                           Real& npv,
                           Real& bps);

        //! At-the-money rate of the cash flows.
        /*! The result is the fixed rate for which a fixed rate cash flow
            vector, equivalent to the input vector, has the required NPV
            according to the given term structure. If the required NPV is
            not given, the input cash flow vector's NPV is used instead.
        */
        static Rate atmRate(const Leg& leg,
                            const YieldTermStructure& discountCurve,
                            bool includeSettlementDateFlows,
                            Date settlementDate = Date(),
                            Date npvDate = Date(),
                            Real npv = Null<Real>());
        //@}

        //! \name Yield (a.k.a. Internal Rate of Return, i.e. IRR) functions
        /*! The IRR is the interest rate at which the NPV of the cash
            flows equals the dirty price.
        */
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given constant interest rate.  The result
            is affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real npv(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        static Real npv(const Leg& leg,
                        Rate yield,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given constant interest rate.  The result is
            affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real bps(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        static Real bps(const Leg& leg,
                        Rate yield,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Implied internal rate of return.
        /*! The function verifies
            the theoretical existence of an IRR and numerically
            establishes the IRR to the desired precision.
        */
        static Rate yield(const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate = Date(),
                          Date npvDate = Date(),
                          Real accuracy = 1.0e-10,
                          Size maxIterations = 100,
                          Rate guess = 0.05);

        template <typename Solver>
        static Rate yield(const Solver& solver,
                          const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate = Date(),
                          Date npvDate = Date(),
                          Real accuracy = 1.0e-10,
                          Rate guess = 0.05) {
            IrrFinder objFunction(leg, npv, dayCounter, compounding,
                                  frequency, includeSettlementDateFlows,
                                  settlementDate, npvDate);
            return solver.solve(objFunction, accuracy, guess, guess/10.0);
        }

        //! Cash-flow duration.
        /*! The simple duration of a string of cash flows is defined as
            \f[
            D_{\mathrm{simple}} = \frac{\sum t_i c_i B(t_i)}{\sum c_i B(t_i)}
            \f]
            where \f$ c_i \f$ is the amount of the \f$ i \f$-th cash
            flow, \f$ t_i \f$ is its payment time, and \f$ B(t_i) \f$
            is the corresponding discount according to the passed yield.

            The modified duration is defined as
            \f[
            D_{\mathrm{modified}} = -\frac{1}{P} \frac{\partial P}{\partial y}
            \f]
            where \f$ P \f$ is the present value of the cash flows
            according to the given IRR \f$ y \f$.

            The Macaulay duration is defined for a compounded IRR as
            \f[
            D_{\mathrm{Macaulay}} = \left( 1 + \frac{y}{N} \right)
                                    D_{\mathrm{modified}}
            \f]
            where \f$ y \f$ is the IRR and \f$ N \f$ is the number of
            cash flows per year.
        */
        static Time duration(const Leg& leg,
                             const InterestRate& yield,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date(),
                             Date npvDate = Date());
        static Time duration(const Leg& leg,
                             Rate yield,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date(),
                             Date npvDate = Date());

        //! Cash-flow convexity
        /*! The convexity of a string of cash flows is defined as
            \f[
            C = \frac{1}{P} \frac{\partial^2 P}{\partial y^2}
            \f]
            where \f$ P \f$ is the present value of the cash flows
            according to the given IRR \f$ y \f$.
        */
        static Real convexity(const Leg& leg,
                              const InterestRate& yield,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date());
        static Real convexity(const Leg& leg,
                              Rate yield,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date());

        //! Basis-point value
        /*! Obtained by setting dy = 0.0001 in the 2nd-order Taylor
            series expansion.
        */
        static Real basisPointValue(const Leg& leg,
                                    const InterestRate& yield,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate = Date(),
                                    Date npvDate = Date());
        static Real basisPointValue(const Leg& leg,
                                    Rate yield,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate = Date(),
                                    Date npvDate = Date());

        //! Yield value of a basis point
        /*! The yield value of a one basis point change in price is
            the derivative of the yield with respect to the price
            multiplied by 0.01
        */
        static Real yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& yield,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate = Date(),
                                         Date npvDate = Date());
        static Real yieldValueBasisPoint(const Leg& leg,
                                         Rate yield,
                                         const DayCounter& dayCounter,
                                         Compounding compounding,
                                         Frequency frequency,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate = Date(),
                                         Date npvDate = Date());
        //@}

        //! \name Z-spread functions
        /*! For details on z-spread refer to:
            "Credit Spreads Explained", Lehman Brothers European Fixed
            Income Research - March 2004, D. O'Kane
        */
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the z-spreaded term structure.  The result
            is affected by the choice of the z-spread compounding
            and the relative frequency and day counter.
        */
        static Real npv(const Leg& leg,
                        const ext::shared_ptr<YieldTermStructure>& discount,
                        Spread zSpread,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! implied Z-spread.
        static Spread zSpread(const Leg& leg,
                              Real npv,
                              const ext::shared_ptr<YieldTermStructure>&,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date(),
                              Real accuracy = 1.0e-10,
                              Size maxIterations = 100,
                              Rate guess = 0.0);
        //! deprecated implied Z-spread.
        static Spread zSpread(const Leg& leg,
                              const ext::shared_ptr<YieldTermStructure>& d,
                              Real npv,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date(),
                              Real accuracy = 1.0e-10,
                              Size maxIterations = 100,
                              Rate guess = 0.0) {
            return zSpread(leg, npv, d, dayCounter, compounding, frequency,
                           includeSettlementDateFlows, settlementDate, npvDate,
                           accuracy, maxIterations, guess);
        }
        //@}

    };

}

#endif
