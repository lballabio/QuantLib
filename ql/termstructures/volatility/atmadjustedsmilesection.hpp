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

/*! \file atmadjustedsmilesection.hpp
    \brief smile section that allows for alternate specification of atm level
   and recentering the source volatility accordingly
*/

#ifndef quantlib_atm_adjusted_smile_section_hpp
#define quantlib_atm_adjusted_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    class AtmAdjustedSmileSection : public SmileSection {

      public:
        explicit AtmAdjustedSmileSection(const ext::shared_ptr<SmileSection>& source,
                                         Real atm = Null<Real>(),
                                         bool recenterSmile = false);

        Real minStrike() const override { return source_->minStrike(); }
        Real maxStrike() const override { return source_->maxStrike(); }
        Real atmLevel() const override { return f_; }
        const Date& exerciseDate() const override { return source_->exerciseDate(); }
        Time exerciseTime() const override { return source_->exerciseTime(); }
        const DayCounter& dayCounter() const override { return source_->dayCounter(); }
        const Date& referenceDate() const override { return source_->referenceDate(); }
        VolatilityType volatilityType() const override { return source_->volatilityType(); }
        Rate shift() const override { return source_->shift(); }

        Real optionPrice(Rate strike,
                         Option::Type type = Option::Call,
                         Real discount = 1.0) const override {
            return source_->optionPrice(adjustedStrike(strike), type, discount);
        }

        Real digitalOptionPrice(Rate strike,
                                Option::Type type = Option::Call,
                                Real discount = 1.0,
                                Real gap = 1.0e-5) const override {
            return source_->digitalOptionPrice(adjustedStrike(strike), type,
                                               discount, gap);
        }

        Real vega(Rate strike, Real discount = 1.0) const override {
            return source_->vega(adjustedStrike(strike), discount);
        }

        Real density(Rate strike, Real discount = 1.0, Real gap = 1.0E-4) const override {
            return source_->density(adjustedStrike(strike), discount, gap);
        }

      protected:
        Real varianceImpl(Rate strike) const override;
        Volatility volatilityImpl(Rate strike) const override;

      private:

        Real adjustedStrike(Real strike) const;
        ext::shared_ptr<SmileSection> source_;
        Real adjustment_;
        Real f_;
    };
}

#endif


#ifndef id_7afe80d23e616ab1c546bcbf747a5cd7
#define id_7afe80d23e616ab1c546bcbf747a5cd7
inline bool test_7afe80d23e616ab1c546bcbf747a5cd7(int* i) { return i != 0; }
#endif
