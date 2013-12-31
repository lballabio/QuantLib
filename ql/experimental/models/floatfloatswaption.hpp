/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file floatfloatswaption.hpp
    \brief floatfloatswaption class
*/

#ifndef quantlib_instruments_floatfloatswaption_hpp
#define quantlib_instruments_floatfloatswaption_hpp

#include <ql/option.hpp>
#include <ql/experimental/models/floatfloatswap.hpp>
#include <ql/experimental/models/basketgeneratingengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/utilities/disposable.hpp>

namespace QuantLib {

    //! floatfloat swaption class
    /*! \ingroup instruments
    */

    class FloatFloatSwaption : public Option {
      public:
        class arguments;
        class engine;
        FloatFloatSwaption(const boost::shared_ptr<FloatFloatSwap> &swap,
                           const boost::shared_ptr<Exercise> &exercise);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments *) const;
        //@}
        //! \name Inspectors
        //@{
        VanillaSwap::Type type() const { return swap_->type(); }
        const boost::shared_ptr<FloatFloatSwap> &underlyingSwap() const {
            return swap_;
        }
        //@}
        Disposable<std::vector<boost::shared_ptr<CalibrationHelper> > >
        calibrationBasket(
            boost::shared_ptr<SwapIndex> standardSwapBase,
            boost::shared_ptr<SwaptionVolatilityStructure> swaptionVolatility,
            const BasketGeneratingEngine::CalibrationBasketType basketType =
                BasketGeneratingEngine::MaturityStrikeByDeltaGamma) const;

      private:
        // arguments
        boost::shared_ptr<FloatFloatSwap> swap_;
    };

    //! %Arguments for cms swaption calculation
    class FloatFloatSwaption::arguments : public FloatFloatSwap::arguments,
                                          public Option::arguments {
      public:
        arguments() {}
        boost::shared_ptr<FloatFloatSwap> swap;
        void validate() const;
    };

    //! base class for cms swaption engines
    class FloatFloatSwaption::engine
        : public GenericEngine<FloatFloatSwaption::arguments,
                               FloatFloatSwaption::results> {};
}

#endif
