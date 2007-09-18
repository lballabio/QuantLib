/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Katiuscia Manzoni

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

/*! \file OptionletStripperAdapter.hpp
    \brief Optionlet Stripper Adapter
*/

#ifndef quantlib_optionlet_stripper_adapter_h
#define quantlib_optionlet_stripper_adapter_h

#include <ql/voltermstructures/interestrate/caplet/optionletvolatilitystructure.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>

namespace QuantLib {

    class OptionletStripper;
    class Interpolation2D;

    class OptionletStripperAdapter : public OptionletVolatilityStructure,
                                     public LazyObject {
      public:
        OptionletStripperAdapter(const Handle<OptionletStripper>& optionletStripper);

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        const Date& referenceDate() const;
        //@}
        //! \name OptionletVolatilityStructure interface
        //@{
        Rate minStrike() const;
        Rate maxStrike() const;
        //@} 
        //! \name LazyObject interface
        //@{
        void update();
        void performCalculations() const;
        //@}

      protected:
        //! \name OptionletVolatilityStructure interface
        //@{
        Volatility volatilityImpl(Time length,
                                  Rate strike) const;
        //@} 
    private:
        const Handle<OptionletStripper> optionletStripper_;
        mutable Interpolation2D interpolation_;
    };

    inline void OptionletStripperAdapter::update() {
        TermStructure::update();
        LazyObject::update();
    }

}

#endif
