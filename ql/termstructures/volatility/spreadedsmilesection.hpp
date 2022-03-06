/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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

/*! \file spreadedsmilesection.hpp
    \brief Spreaded SmileSection class
*/

#ifndef quantlib_spreaded_smile_section_hpp
#define quantlib_spreaded_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class Quote;

    class SpreadedSmileSection : public SmileSection {
      public:
        SpreadedSmileSection(ext::shared_ptr<SmileSection>, Handle<Quote> spread);
        //! \name SmileSection interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        Real atmLevel() const override;
        const Date& exerciseDate() const override;
        Time exerciseTime() const override;
        const DayCounter& dayCounter() const override;
        const Date& referenceDate() const override;
        VolatilityType volatilityType() const override;
        Rate shift() const override;
        //@}
        //! \name LazyObject interface
        //@{
        void update() override { notifyObservers(); }
        //@}
      protected:
        Volatility volatilityImpl(Rate strike) const override;

      private:
        const ext::shared_ptr<SmileSection> underlyingSection_;
        const Handle<Quote> spread_;
    };

    inline Real SpreadedSmileSection::minStrike() const {
        return underlyingSection_->minStrike();
    }

    inline Real SpreadedSmileSection::maxStrike() const {
        return underlyingSection_->maxStrike();
    }

    inline Real SpreadedSmileSection::atmLevel() const {
        return underlyingSection_->atmLevel();
    }

    inline const Date& SpreadedSmileSection::exerciseDate() const {
        return underlyingSection_->exerciseDate();
    }

    inline Time SpreadedSmileSection::exerciseTime() const {
        return underlyingSection_->exerciseTime();
    }

    inline const DayCounter& SpreadedSmileSection::dayCounter() const {
        return underlyingSection_->dayCounter();
    }

    inline const Date& SpreadedSmileSection::referenceDate() const {
        return underlyingSection_->referenceDate();
    }

    inline VolatilityType SpreadedSmileSection::volatilityType() const {
        return underlyingSection_->volatilityType();
    }

    inline Rate SpreadedSmileSection::shift() const {
        return underlyingSection_->shift();
    }
}

#endif


#ifndef id_5c742fb71cf4b16e7fd680da258cb193
#define id_5c742fb71cf4b16e7fd680da258cb193
inline bool test_5c742fb71cf4b16e7fd680da258cb193(const int* i) {
    return i != nullptr;
}
#endif
