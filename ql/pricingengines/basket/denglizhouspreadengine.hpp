/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file denglizhoubasketengine.hpp
    \brief Deng, Li and Zhou: Closed-Form Approximation for Spread option pricing
*/

#ifndef quantlib_deng_li_zhou_spread_engine_hpp
#define quantlib_deng_li_zhou_spread_engine_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for basket option on two futures
    /*! This class implements the pricing formula from
        "Multi-asset Spread Option Pricing and Hedging",
        S. Deng, M. Li, J.Zhou, 2008
        https://mpra.ub.uni-muenchen.de/8259/1/MPRA_paper_8259.pdf

        The Typo in formula (37) for J^2 is corrected

        This pricing formula works only if exactly one asset weight is positive.
        If more than one weight is positive then a mapping of the sum of correlated
        log-normal processes onto one log-normal process has to has be carried out.
        This implementation is using

        "WKB Approximation for the Sum of Two Correlated Lognormal Random Variables",
        C.F. Lo 2013
        https://www.m-hikari.com/ams/ams-2013/ams-125-128-2013/loAMS125-128-2013.pdf

        for this task.

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class DengLiZhouSpreadEngine : public BasketOption::engine {
      public:
        DengLiZhouSpreadEngine(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
            Matrix rho);

        void calculate() const override;

      private:
        static Real calculate_vanilla_call(
            const Array& s, DiscountFactor dr, const Array& dq,
            const Array& v, const Matrix& rho, Time T);

        static Real I(Real u, Real tF2, const Matrix& D, const Matrix& DF, Size i);

        const Size n_;
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
        const Matrix rho_;
    };
}

#endif
