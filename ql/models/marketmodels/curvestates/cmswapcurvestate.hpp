/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Mark Joshi

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


#ifndef quantlib_cmswapcurvestate_hpp
#define quantlib_cmswapcurvestate_hpp

#include <ql/models/marketmodels/curvestate.hpp>

namespace QuantLib {

    //! %Curve state for constant-maturity-swap market models
    class CMSwapCurveState : public CurveState {
      public:
        explicit CMSwapCurveState(const std::vector<Time>& rateTimes,
                                  Size spanningForwards);

        //! \name Modifiers
        //@{
        void setOnCMSwapRates(const std::vector<Rate>& cmSwapRates,
                              Size firstValidIndex = 0);
        //@}
        //! \name Inspectors
        //@{
        Real discountRatio(Size i, Size j) const override;
        Rate forwardRate(Size i) const override;

        Rate coterminalSwapRate(Size i) const override;
        Rate coterminalSwapAnnuity(Size numeraire, Size i) const override;

        Rate cmSwapRate(Size i, Size spanningForwards) const override;
        Rate cmSwapAnnuity(Size numeraire, Size i, Size spanningForwards) const override;
        const std::vector<Rate>& forwardRates() const override;
        const std::vector<Rate>& coterminalSwapRates() const override;
        const std::vector<Rate>& cmSwapRates(Size spanningForwards) const override;
#if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<CurveState> clone() const override;
#else
        std::auto_ptr<CurveState> clone() const;
        #endif
        //@}
      private:
        Size spanningFwds_;
        Size first_;
        std::vector<DiscountFactor> discRatios_;
        mutable std::vector<Rate> forwardRates_;
        // fixed number of spanning forwards
        std::vector<Rate> cmSwapRates_;
        std::vector<Real> cmSwapAnnuities_;
        // irregular number of spanning forwards
        mutable std::vector<Rate> irrCMSwapRates_;
        mutable std::vector<Real> irrCMSwapAnnuities_;
        mutable std::vector<Rate> cotSwapRates_;
        mutable std::vector<Real> cotAnnuities_;
    };

}

#endif
