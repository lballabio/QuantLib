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

/*! \file strippedoptionletadapter.hpp
    \brief StrippedOptionlet Adapter
*/

#ifndef quantlib_stripped_optionlet_adapter_h
#define quantlib_stripped_optionlet_adapter_h

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>

namespace QuantLib {

    /*! Adapter class for turning a StrippedOptionletBase object into an
        OptionletVolatilityStructure.
    */
    class StrippedOptionletAdapter : public OptionletVolatilityStructure,
                                     public LazyObject {
      public:
          StrippedOptionletAdapter(
                              const boost::shared_ptr<StrippedOptionletBase>&);

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name VolatilityTermStructure interface
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
        boost::shared_ptr<SmileSection> smileSectionImpl(
                                                Time optionTime) const;
        Volatility volatilityImpl(Time length,
                                  Rate strike) const;
        //@} 
    private:
        const boost::shared_ptr<StrippedOptionletBase> optionletStripper_;
        Size nInterpolations_;
        mutable std::vector<boost::shared_ptr<Interpolation> > strikeInterpolations_;
    };

    inline void StrippedOptionletAdapter::update() {
        TermStructure::update();
        LazyObject::update();
    }

}

#endif
