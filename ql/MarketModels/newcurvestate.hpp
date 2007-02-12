/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_curvestate_hpp
#define quantlib_curvestate_hpp

#include <ql/Math/array.hpp>
#include <vector>

namespace QuantLib {

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
        template <class ForwardIterator>
        CurveState(ForwardIterator begin, ForwardIterator end)
        : rateTimes_(begin, end), taus_(rateTimes_.size()),
          nRates_(rateTimes_.size()-1)
        {
            for (Size i=0; i<nRates_; ++i)
                taus_[i] = rateTimes_[i+1] - rateTimes_[i];
        }

        //! \name Inspectors
        //@{
        Size numberOfRates() const { return nRates_; }

        const std::vector<Time>& rateTimes() const { return rateTimes_; }
        const std::vector<Time>& rateTaus() const { return taus_; }

        Real discountRatio(Size i, Size j) const;

        virtual const std::vector<Rate>& forwardRates() const = 0;
        virtual const std::vector<DiscountFactor>& discountRatios() const = 0;
        virtual const std::vector<Real>& coterminalSwapAnnuities() const = 0;
        virtual const std::vector<Rate>& coterminalSwapRates() const = 0;
        virtual const std::vector<Real>& cmSwapAnnuities(Size spanningForwards) const = 0;
        virtual const std::vector<Rate>& cmSwapRates(Size spanningForwards) const = 0;

        virtual Rate forwardRate(Size i) const = 0;
        virtual Rate coterminalSwapAnnuity(Size i) const = 0;
        virtual Rate coterminalSwapRate(Size i) const = 0;
        virtual Rate cmSwapAnnuity(Size i,
                                 Size spanningForwards) const = 0;
        virtual Rate cmSwapRate(Size i,
                              Size spanningForwards) const = 0;

        //@}
      protected:
        std::vector<Time> rateTimes_, taus_;
        Size nRates_;
    };

    inline Real CurveState::discountRatio(Size i, Size j) const {
        Size iMin = std::min(i, j);
        QL_REQUIRE(iMin>=first_, "index too low");
        QL_REQUIRE(std::max(i, j)<=nRates_, "index too high");
        return discRatios_[i]/discRatios_[j];
    }

}

#endif
