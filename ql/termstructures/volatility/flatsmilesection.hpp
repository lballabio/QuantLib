/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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
                         Real atmLevel = Null<Rate>(),
                         VolatilityType type = ShiftedLognormal,
                         Real shift = 0.0);
        FlatSmileSection(Time exerciseTime,
                         Volatility vol,
                         const DayCounter& dc,
                         Real atmLevel = Null<Rate>(),
                         VolatilityType type = ShiftedLognormal,
                         Real shift = 0.0);
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        Real atmLevel() const override;
        //@}
      protected:
        Volatility volatilityImpl(Rate) const override;

      private:
        Volatility vol_;
        Real atmLevel_;
    };

    inline Real FlatSmileSection::minStrike () const {
        return QL_MIN_REAL - shift();
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


#ifndef id_2bfb7e3a548b8d31ba2e8837dded776c
#define id_2bfb7e3a548b8d31ba2e8837dded776c
inline bool test_2bfb7e3a548b8d31ba2e8837dded776c(const int* i) {
    return i != nullptr;
}
#endif
