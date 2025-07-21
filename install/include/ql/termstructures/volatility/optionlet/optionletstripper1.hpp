/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Katiuscia Manzoni
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2015 Michael von den Driesch

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

/*! \file optionletstripper1.hpp
    \brief optionlet (caplet/floorlet) volatility stripper
*/

#ifndef quantlib_optionletstripper1_hpp
#define quantlib_optionletstripper1_hpp

#include <ql/termstructures/volatility/optionlet/optionletstripper.hpp>

namespace QuantLib {

    class SimpleQuote;
    class CapFloor;
    class PricingEngine;

    /*! Helper class to strip optionlet (i.e. caplet/floorlet) volatilities
        (a.k.a. forward-forward volatilities) from the (cap/floor) term
        volatilities of a CapFloorTermVolSurface.
    */
    class OptionletStripper1 : public OptionletStripper {
      public:
        OptionletStripper1(
            const ext::shared_ptr<CapFloorTermVolSurface>&,
            const ext::shared_ptr<IborIndex>& index,
            Rate switchStrikes = Null<Rate>(),
            Real accuracy = 1.0e-6,
            Natural maxIter = 100,
            const Handle<YieldTermStructure>& discount = {},
            VolatilityType type = ShiftedLognormal,
            Real displacement = 0.0,
            bool dontThrow = false,
            ext::optional<Period> optionletFrequency = ext::nullopt);

        const Matrix& capFloorPrices() const;
        const Matrix &capletVols() const;
        const Matrix& capFloorVolatilities() const;
        const Matrix& optionletPrices() const;
        Rate switchStrike() const;

        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
      private:
        mutable Matrix capFloorPrices_, optionletPrices_;
        mutable Matrix capFloorVols_;
        mutable Matrix optionletStDevs_, capletVols_;

        bool floatingSwitchStrike_;

        mutable Rate switchStrike_;
        Real accuracy_;
        Natural maxIter_;
        bool dontThrow_;
    };

}

#endif
