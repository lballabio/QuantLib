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
    /*! This class implements formulae from
        "Multi-asset Spread Option Pricing and Hedging",
        S. Deng, M. Li, J.Zhou,
        https://mpra.ub.uni-muenchen.de/8259/1/MPRA_paper_8259.pdf
        Typo in formula (37) for J^2 is corrected.
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

        static Real calculate_vanilla_call(
            const Array& s, const Array& dr, const Array& dq,
            const Array& v, const Matrix& rho, Time T);

      private:
        const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
        const Matrix rho_;
    };
}

#endif
