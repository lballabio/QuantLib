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


#ifndef quantlib_coterminalswapcurvestate_hpp
#define quantlib_coterminalswapcurvestate_hpp

#include <ql/MarketModels/newcurvestate.hpp>

namespace QuantLib {

    class CoterminalSwapCurveState : public NewCurveState {
      public:
        CoterminalSwapCurveState(const std::vector<Time>& rateTimes)
        : NewCurveState(rateTimes) {}

        //! \name Modifiers
        //@{
        void setOnCoterminalSwapRates(const std:vector<Rate>& swapRates,
                            Size firstValidIndex = 0) {
            QL_REQUIRE(swapRates.size()==nRates_,
                       "rates mismatch: " <<
                       nRates_ << " required, " <<
                       swapRates.size() << " provided");
            QL_REQUIRE(firstValidIndex<nRates_,
                       "first valid index must be less than " <<
                       nRates_ << ": " <<
                       firstValidIndex << " not allowed");

            // implementation here

        }
        //@}

        //! \name Inspectors
        //@{
        virtual const std::vector<Rate>& forwardRates() const;
        virtual const std::vector<DiscountFactor>& discountRatios() const;
        virtual const std::vector<Real>& coterminalSwapAnnuities() const;
        virtual const std::vector<Rate>& coterminalSwapRates() const;
        virtual const std::vector<Real>& cmSwapAnnuities(Size spanningForwards) const;
        virtual const std::vector<Rate>& cmSwapRates(Size spanningForwards) const;

        virtual Rate forwardRate(Size i) const;
        virtual Rate coterminalSwapAnnuity(Size i) const;
        virtual Rate coterminalSwapRate(Size i) const;
        virtual Rate cmSwapAnnuity(Size i,
                                 Size spanningForwards) const;
        virtual Rate cmSwapRate(Size i,
                              Size spanningForwards) const;
        //@}
      private:
    };

}

#endif
