/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file flatsmilesection.hpp
    \brief Flat SmileSection
*/

#ifndef quantlib_flat_smile_section_hpp
#define quantlib_flat_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    class FlatSmileSection : public SmileSection {
      public:
        FlatSmileSection(const Date& d,
                         Volatility vol,
                         const DayCounter& dc,
                         const Date& referenceDate = Date(),
                         Real atmLevel = Null<Rate>());
        FlatSmileSection(Time exerciseTime,
                         Volatility vol,
                         const DayCounter& dc,
                         Real atmLevel = Null<Rate>());
        //! \name SmileSection interface
        //@{
        Real minStrike () const;
        Real maxStrike () const;
        Real atmLevel() const;
        //@}
      protected:
        Volatility volatilityImpl(Rate) const;
      private:
        Volatility vol_;
        Real atmLevel_;
    };

    inline Real FlatSmileSection::minStrike () const {
        return QL_MIN_REAL;
    }

    inline Real FlatSmileSection::maxStrike () const {
        return QL_MAX_REAL;
    }

    inline Real FlatSmileSection::atmLevel() const {
        return atmLevel_;
    }

    inline Volatility FlatSmileSection::volatilityImpl(Rate) const {
        return vol_;
    }

}

#endif
