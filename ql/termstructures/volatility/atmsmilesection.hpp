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

/*! \file atmsmilesection.hpp
    \brief smile section that allows for explicit / alternate specification of
   atm level
*/

#ifndef quantlib_atm_smile_section_hpp
#define quantlib_atm_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    class AtmSmileSection : public SmileSection {

      public:
        AtmSmileSection(const ext::shared_ptr<SmileSection>& source, Real atm = Null<Real>());

        Real minStrike() const override { return source_->minStrike(); }
        Real maxStrike() const override { return source_->maxStrike(); }
        Real atmLevel() const override { return f_; }
        const Date& exerciseDate() const override { return source_->exerciseDate(); }
        Time exerciseTime() const override { return source_->exerciseTime(); }
        const DayCounter& dayCounter() const override { return source_->dayCounter(); }
        const Date& referenceDate() const override { return source_->referenceDate(); }
        VolatilityType volatilityType() const override { return source_->volatilityType(); }
        Rate shift() const override { return source_->shift(); }

      protected:
        Volatility volatilityImpl(Rate strike) const override {
            return source_->volatility(strike);
        }
        Real varianceImpl(Rate strike) const override { return source_->variance(strike); }

      private:
        ext::shared_ptr<SmileSection> source_;
        Real f_;
    };
}

#endif


#ifndef id_363a14d3598edb04b0f1dee292a3be2c
#define id_363a14d3598edb04b0f1dee292a3be2c
inline bool test_363a14d3598edb04b0f1dee292a3be2c(const int* i) {
    return i != nullptr;
}
#endif
