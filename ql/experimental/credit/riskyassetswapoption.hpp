/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

/*! \file riskyassetswapoption.hpp
    \brief option on risky asset swap
*/

#ifndef quantlib_risky_asset_swap_option_hpp
#define quantlib_risky_asset_swap_option_hpp

#include <ql/experimental/credit/riskyassetswap.hpp>

namespace QuantLib {

    //! %Option on risky asset swap
    /*! \ingroup credit */
    class RiskyAssetSwapOption : public Instrument {
      public:
        RiskyAssetSwapOption(ext::shared_ptr<RiskyAssetSwap> asw,
                             const Date& expiry,
                             Rate marketSpread,
                             Volatility spreadVolatility);

      private:
        bool isExpired() const override;
        void performCalculations() const override;

        ext::shared_ptr<RiskyAssetSwap> asw_;
        Date expiry_;
        Rate marketSpread_;
        Volatility spreadVolatility_;
    };

}

#endif
