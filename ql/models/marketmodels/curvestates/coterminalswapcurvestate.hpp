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


#ifndef quantlib_coterminalswapcurvestate_hpp
#define quantlib_coterminalswapcurvestate_hpp

#include <ql/models/marketmodels/curvestate.hpp>

namespace QuantLib {

    //! %Curve state for coterminal-swap market models
    /*! This class stores the state of the yield curve associated to the
        fixed calendar times within the simulation.
        This is the workhorse discounting object associated to the rate times
        of the simulation. It's important to pass the rates via an object like
        this to the product rather than directly to make it easier to switch
        to other engines such as a coterminal swap rate engine.
        Many products will not need expired rates and others will only require
        the first rate.
    */
    class CoterminalSwapCurveState : public CurveState {
    /* There will n+1 rate times expressing payment and reset times
        of coterminal swap rates.

                |-----|-----|-----|-----|-----|      (size = 6)
                0     1     2     3     4     5      index (i)      (i = 0, ..., 5)
                t0    t1    t2    t3    t4    t5     rateTimes      (i = 0, ..., 5; dim = 6)
                tau0  tau1  tau2  tau3  tau4         rateTaus       (i = 0, ..., 4; dim = 5)
                f0    f1    f2    f3    f4           forwardRates   (i = 0, ..., 4; dim = 5)
                d0    d1    d2    d3    d4    d5     discountBonds  (i = 0, ..., 5; dim = 6)
                d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios (i = 0, ..., 5; dim = 6)
                sr0   sr1   sr2   sr3   sr4          cotSwapsRates  (i = 0, ..., 4; dim = 5)
                a0    a1    a2    a3    a4           cotAnnuities   (i = 0, ..., 4; dim = 5)
    */
      public:
        explicit CoterminalSwapCurveState(const std::vector<Time>& rateTimes);
        //! \name Modifiers
        //@{
        void setOnCoterminalSwapRates(const std::vector<Rate>& swapRates,
                                      Size firstValidIndex = 0);
        //@}

        //! \name Inspectors
        //@{
        Real discountRatio(Size i, Size j) const;
        Rate forwardRate(Size i) const;
        Rate coterminalSwapRate(Size i) const;
        Rate coterminalSwapAnnuity(Size numeraire, Size i) const;
        Rate cmSwapRate(Size i, Size spanningForwards) const;
        Rate cmSwapAnnuity(Size numeraire,
                           Size i,
                           Size spanningForwards) const;
        const std::vector<Rate>& forwardRates() const;
        const std::vector<Rate>& coterminalSwapRates() const;
        const std::vector<Rate>& cmSwapRates(Size spanningForwards) const;
        //@}
        std::auto_ptr<CurveState> clone() const {
            return std::auto_ptr<CurveState>(
                new CoterminalSwapCurveState(*this));
        }
      private:
        Size first_;
        std::vector<DiscountFactor> discRatios_;
        mutable std::vector<Rate> forwardRates_;
        mutable std::vector<Rate> cmSwapRates_;
        mutable std::vector<Real> cmSwapAnnuities_;
        std::vector<Rate> cotSwapRates_;
        std::vector<Real> cotAnnuities_;
    };

}

#endif
