/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2018 Peter Caspers

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

/*! \file nonstandardswaption.hpp
    \brief nonstandard swap option class
*/

#ifndef quantlib_instruments_nonstandardswaption_hpp
#define quantlib_instruments_nonstandardswaption_hpp

#include <ql/option.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/instruments/nonstandardswap.hpp>
#include <ql/pricingengines/swaption/basketgeneratingengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/utilities/disposable.hpp>

namespace QuantLib {

    //! nonstandard swaption class
    /*! \ingroup instruments
    */

    class NonstandardSwaption : public Option {
      public:
        class arguments;
        class engine;
        NonstandardSwaption(const Swaption &fromSwaption);
        NonstandardSwaption(ext::shared_ptr<NonstandardSwap> swap,
                            const ext::shared_ptr<Exercise>& exercise,
                            Settlement::Type delivery = Settlement::Physical,
                            Settlement::Method settlementMethod = Settlement::PhysicalOTC);

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        Settlement::Type settlementType() const { return settlementType_; }
        Settlement::Method settlementMethod() const {
            return settlementMethod_;
        }
        Swap::Type type() const { return swap_->type(); }

        const ext::shared_ptr<NonstandardSwap> &underlyingSwap() const {
            return swap_;
        }
        //@}
        Disposable<std::vector<ext::shared_ptr<BlackCalibrationHelper> > >
        calibrationBasket(const ext::shared_ptr<SwapIndex>& standardSwapBase,
                          const ext::shared_ptr<SwaptionVolatilityStructure>& swaptionVolatility,
                          BasketGeneratingEngine::CalibrationBasketType basketType =
                              BasketGeneratingEngine::MaturityStrikeByDeltaGamma) const;

      private:
        // arguments
        ext::shared_ptr<NonstandardSwap> swap_;
        Settlement::Type settlementType_;
        Settlement::Method settlementMethod_;
    };

    //! %Arguments for nonstandard swaption calculation
    class NonstandardSwaption::arguments : public NonstandardSwap::arguments,
                                           public Option::arguments {
      public:
        arguments() = default;
        ext::shared_ptr<NonstandardSwap> swap;
        Settlement::Type settlementType;
        Settlement::Method settlementMethod;
        void validate() const override;
    };

    //! base class for nonstandard swaption engines
    class NonstandardSwaption::engine
        : public GenericEngine<NonstandardSwaption::arguments,
                               NonstandardSwaption::results> {};
}

#endif


#ifndef id_2880a2daab5a25624e3bcd3436a68f6c
#define id_2880a2daab5a25624e3bcd3436a68f6c
inline bool test_2880a2daab5a25624e3bcd3436a68f6c(const int* i) {
    return i != nullptr;
}
#endif
