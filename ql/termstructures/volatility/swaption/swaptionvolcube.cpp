/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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

#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    SwaptionVolatilityCube::SwaptionVolatilityCube(
        const Handle<SwaptionVolatilityStructure>& atmVol,
        const std::vector<Period>& optionTenors,
        const std::vector<Period>& swapTenors,
        const std::vector<Spread>& strikeSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volSpreads,
        const boost::shared_ptr<SwapIndex>& swapIndexBase,
        const boost::shared_ptr<SwapIndex>& shortSwapIndexBase,
        bool vegaWeightedSmileFit)
    : SwaptionVolatilityDiscrete(optionTenors, swapTenors, 0,
                                 atmVol->calendar(),
                                 atmVol->businessDayConvention(),
                                 atmVol->dayCounter()),
      atmVol_(atmVol),
      nStrikes_(strikeSpreads.size()),
      strikeSpreads_(strikeSpreads),
      localStrikes_(nStrikes_),
      localSmile_(nStrikes_),
      volSpreads_(volSpreads),
      swapIndexBase_(swapIndexBase),
      shortSwapIndexBase_(shortSwapIndexBase),
      vegaWeightedSmileFit_(vegaWeightedSmileFit)
    {
        QL_REQUIRE(!atmVol.empty(), "atm vol handle not linked to anything");

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (Size i=1; i<nStrikes_; ++i)
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                       "non increasing strike spreads: " <<
                       io::ordinal(i) << " is " << strikeSpreads_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << strikeSpreads_[i]);

        QL_REQUIRE(!volSpreads_.empty(), "empty vol spreads matrix");

        QL_REQUIRE(nOptionTenors_*nSwapTenors_==volSpreads_.size(),
            "mismatch between number of option tenors * swap tenors (" <<
            nOptionTenors_*nSwapTenors_ << ") and number of rows (" <<
            volSpreads_.size() << ")");

        for (Size i=0; i<volSpreads_.size(); i++)
            QL_REQUIRE(nStrikes_==volSpreads_[i].size(),
                       "mismatch between number of strikes (" << nStrikes_ <<
                       ") and number of columns (" << volSpreads_[i].size() <<
                       ") in the " << io::ordinal(i+1) << " row");

        registerWith(atmVol_);
        atmVol_->enableExtrapolation();

        registerWith(swapIndexBase_);
        registerWith(shortSwapIndexBase_);

        QL_REQUIRE(shortSwapIndexBase_->tenor()<swapIndexBase_->tenor(),
                   "short index tenor (" << shortSwapIndexBase_->tenor() <<
                   ") is not less than index tenor (" <<
                   swapIndexBase_->tenor() << ")");

        registerWithVolatilitySpread();
        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

    void SwaptionVolatilityCube::registerWithVolatilitySpread()
    {
        for (Size i=0; i<nStrikes_; i++)
            for (Size j=0; j<nOptionTenors_; j++)
                for (Size k=0; k<nSwapTenors_; k++)
                    registerWith(volSpreads_[j*nSwapTenors_+k][i]);
    }

    Rate SwaptionVolatilityCube::atmStrike(const Date& optionD,
                                           const Period& swapTenor) const {

        // FIXME use a familyName-based index factory
        if (swapTenor > shortSwapIndexBase_->tenor()) {
            if (swapIndexBase_->exogenousDiscount()) {
                return SwapIndex(swapIndexBase_->familyName(),
                                 swapTenor,
                                 swapIndexBase_->fixingDays(),
                                 swapIndexBase_->currency(),
                                 swapIndexBase_->fixingCalendar(),
                                 swapIndexBase_->fixedLegTenor(),
                                 swapIndexBase_->fixedLegConvention(),
                                 swapIndexBase_->dayCounter(),
                                 swapIndexBase_->iborIndex(),
                                 swapIndexBase_->discountingTermStructure())
                    .fixing(optionD);
            } else {
                return SwapIndex(swapIndexBase_->familyName(),
                                 swapTenor,
                                 swapIndexBase_->fixingDays(),
                                 swapIndexBase_->currency(),
                                 swapIndexBase_->fixingCalendar(),
                                 swapIndexBase_->fixedLegTenor(),
                                 swapIndexBase_->fixedLegConvention(),
                                 swapIndexBase_->dayCounter(),
                                 swapIndexBase_->iborIndex())
                    .fixing(optionD);
            }
        } else {
            if (shortSwapIndexBase_->exogenousDiscount()) {
                return SwapIndex(shortSwapIndexBase_->familyName(),
                                 swapTenor,
                                 shortSwapIndexBase_->fixingDays(),
                                 shortSwapIndexBase_->currency(),
                                 shortSwapIndexBase_->fixingCalendar(),
                                 shortSwapIndexBase_->fixedLegTenor(),
                                 shortSwapIndexBase_->fixedLegConvention(),
                                 shortSwapIndexBase_->dayCounter(),
                                 shortSwapIndexBase_->iborIndex(),
                                 shortSwapIndexBase_->discountingTermStructure())
                    .fixing(optionD);
            } else {
                return SwapIndex(shortSwapIndexBase_->familyName(),
                                 swapTenor,
                                 shortSwapIndexBase_->fixingDays(),
                                 shortSwapIndexBase_->currency(),
                                 shortSwapIndexBase_->fixingCalendar(),
                                 shortSwapIndexBase_->fixedLegTenor(),
                                 shortSwapIndexBase_->fixedLegConvention(),
                                 shortSwapIndexBase_->dayCounter(),
                                 shortSwapIndexBase_->iborIndex())
                    .fixing(optionD);
            }
        }
    }

}
