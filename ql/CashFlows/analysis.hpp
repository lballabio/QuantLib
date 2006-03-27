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

/*! \file analysis.hpp
    \brief Cash-flow analysis functions
*/

#ifndef quantlib_cash_flow_analysis_hpp
#define quantlib_cash_flow_analysis_hpp

#include <ql/cashflow.hpp>
#include <ql/interestrate.hpp>
#include <ql/yieldtermstructure.hpp>
#include <vector>

namespace QuantLib {

    //! duration type
    struct Duration {
        enum Type { Simple, Macaulay, Modified };
    };

    //! cashflows analysis functions
    /*! \todo add tests */
    class Cashflows {
      private:
        Cashflows();
        Cashflows(const Cashflows&);
      public:
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given term structure.
        */
        static Real npv(const std::vector<boost::shared_ptr<CashFlow> >&,
                        const Handle<YieldTermStructure>&);
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given constant interest rate.  The result
            is affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real npv(const std::vector<boost::shared_ptr<CashFlow> >&,
                        const InterestRate&,
                        Date settlementDate = Date());

        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given term structure.
        */
        static Real bps(const std::vector<boost::shared_ptr<CashFlow> >&,
                        const Handle<YieldTermStructure>&);
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given constant interest rate.  The result is
            affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real bps(const std::vector<boost::shared_ptr<CashFlow> >&,
                        const InterestRate&,
                        Date settlementDate = Date());

        //! Internal rate of return.
        /*! The IRR is the interest rate at which the NPV of the cash
            flows equals the given market price. The function verifies
            the theoretical existance of an IRR and numerically
            establishes the IRR to the desired precision.
        */
        static Rate irr(const std::vector<boost::shared_ptr<CashFlow> >&,
                        Real marketPrice,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency = NoFrequency,
                        Date settlementDate = Date(),
                        Real tolerance = 1.0e-10,
                        Size maxIterations = 10000,
                        Rate guess = 0.05);

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
        static Time duration(const std::vector<boost::shared_ptr<CashFlow> >&,
                             const InterestRate& y,
                             Duration::Type type = Duration::Modified,
                             Date settlementDate = Date());


        //! Cash-flow convexity
        /*! The convexity of a string of cash flows is defined as
            \f[
            C = \frac{1}{P} \frac{\partial^2 P}{\partial y^2}
            \f]
            where \f$ P \f$ is the present value of the cash flows
            according to the given IRR \f$ y \f$.
        */
        static Real convexity(const std::vector<boost::shared_ptr<CashFlow> >&,
                              const InterestRate& y,
                              Date settlementDate = Date());
    };

}


#endif
