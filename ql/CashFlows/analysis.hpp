/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Charles Whitmore

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
        /*! THe NPV is the sum of the cash flows, each discounted
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

        //! Cash-flow convexity
        /*! The convexity is defined as
            \f[
            C = \sum t^2 c_t P_t
            \f]
            where \f$ c_t \f$ is the amount of the cash flow and \f$
            P_t \f$ is the discount at time \f$ t \f$ as implied by
            the given interest rate.
        */
        static Real convexity(const std::vector<boost::shared_ptr<CashFlow> >&,
                              const InterestRate&,
                              Date settlementDate = Date());

        //! Cash-flow duration.
        /*! The simple duration is defined as
            \f[
            D_{\mathrm{simple}} = \frac{\sum t c_t P_t}{\sum c_t P_t}
            \f]
            where \f$ c_t \f$ is the amount of the cash flow and \f$
            P_t \f$ is the discount at time \f$ t \f$ as implied by
            the given interest rate.

            The modified duration is
            \f[
            D_{\mathrm{modified}} = \frac{1}{y} D_{\mathrm{simple}}
            \f]
            where \f$ y \f$ is the IRR.

            Finally, the Macaulay duration is
            \f[
            D_{\mathrm{Macaulay}} = \frac{\sum t c_t P'_t}{\sum c_t P'_t}
            \f]
            where \f$ P'_t = e^{-yt} \f$ and \f$ y \f$ is the IRR.
        */
        static Time duration(const std::vector<boost::shared_ptr<CashFlow> >&,
                             Real marketPrice,
                             const InterestRate&,
                             Duration::Type type = Duration::Simple,
                             Date settlementDate = Date());
    };

}


#endif
