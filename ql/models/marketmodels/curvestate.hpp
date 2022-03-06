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


#ifndef quantlib_curve_state_hpp
#define quantlib_curve_state_hpp

#include <ql/math/array.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    //! %Curve state for market-model simulations
    /*! This class stores the state of the yield curve associated to the
        fixed calendar times within the simulation.
        This is the workhorse discounting object associated to the rate times
        of the simulation. It's important to pass the rates via an object like
        this to the product rather than directly to make it easier to switch
        to other engines such as a coterminal swap rate engine.
        Many products will not need expired rates and others will only require
        the first rate.
    */
    class CurveState {
    /* There will n+1 rate times expressing payment and reset times
        of forward rates.

                |-----|-----|-----|-----|-----|      (size = 6)
                t0    t1    t2    t3    t4    t5     rateTimes
                f0    f1    f2    f3    f4           forwardRates
                d0    d1    d2    d3    d4    d5     discountBonds
                d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
                sr0   sr1   sr2   sr3   sr4          cotSwaps
    */
      public:
        CurveState(const std::vector<Time>& rateTimes);
        virtual ~CurveState() = default;

        //! \name Inspectors
        //@{
        Size numberOfRates() const { return numberOfRates_; }

        const std::vector<Time>& rateTimes() const { return rateTimes_; }
        const std::vector<Time>& rateTaus() const { return rateTaus_; }

        virtual Real discountRatio(Size i,
                                   Size j) const = 0;
        virtual Rate forwardRate(Size i) const = 0;
        virtual Rate coterminalSwapAnnuity(Size numeraire,
                                           Size i) const = 0;
        virtual Rate coterminalSwapRate(Size i) const = 0;
        virtual Rate cmSwapAnnuity(Size numeraire,
                                   Size i,
                                   Size spanningForwards) const = 0;
        virtual Rate cmSwapRate(Size i,
                                Size spanningForwards) const = 0;

        virtual const std::vector<Rate>& forwardRates() const = 0;
        virtual const std::vector<Rate>& coterminalSwapRates() const = 0;
        virtual const std::vector<Rate>& cmSwapRates(Size spanningForwards) const = 0;
        Rate swapRate(Size begin,
                      Size end) const;

        #if defined(QL_USE_STD_UNIQUE_PTR)
        virtual std::unique_ptr<CurveState> clone() const = 0;
        #else
        virtual std::auto_ptr<CurveState> clone() const = 0;
        #endif
        //@}
      protected:
        Size numberOfRates_;
        std::vector<Time> rateTimes_, rateTaus_;
    };

    void forwardsFromDiscountRatios(Size firstValidIndex,
                                    const std::vector<DiscountFactor>& ds,
                                    const std::vector<Time>& taus,
                                    std::vector<Rate>& fwds);

    void coterminalFromDiscountRatios(Size firstValidIndex,
                                      const std::vector<DiscountFactor>& ds,
                                      const std::vector<Time>& taus,
                                      std::vector<Rate>& cotSwapRates,
                                      std::vector<Real>& cotSwapAnnuities);

    void constantMaturityFromDiscountRatios( // Size i, // to be added later
        Size spanningForwards,
        Size firstValidIndex,
        const std::vector<DiscountFactor>& ds,
        const std::vector<Time>& taus,
        std::vector<Rate>& cotSwapRates,
        std::vector<Real>& cotSwapAnnuities);
}

#endif


#ifndef id_da11a9b448bc84bb45b3df7e8a308f95
#define id_da11a9b448bc84bb45b3df7e8a308f95
inline bool test_da11a9b448bc84bb45b3df7e8a308f95(const int* i) {
    return i != nullptr;
}
#endif
