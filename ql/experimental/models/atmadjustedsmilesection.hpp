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

        AtmAdjustedSmileSection(const boost::shared_ptr<SmileSection> source,
                                const Real atm = Null<Real>(),
                                bool recenterSmile = false);

        Real minStrike() const { return source_->minStrike(); }
        Real maxStrike() const { return source_->maxStrike(); }
        Real atmLevel() const { return f_; }

        Real optionPrice(Rate strike, Option::Type type = Option::Call,
                         Real discount = 1.0) const {
            return source_->optionPrice(adjustedStrike(strike), type, discount);
        }

        Real digitalOptionPrice(Rate strike, Option::Type type = Option::Call,
                                Real discount = 1.0, Real gap = 1.0E-8) const {
            return source_->digitalOptionPrice(adjustedStrike(strike), type,
                                               discount, gap);
        }

        Real vega(Rate strike, Real discount = 1.0) const {
            return source_->vega(adjustedStrike(strike), discount);
        }

        Real density(Rate strike, Real discount = 1.0,
                     Real gap = 1.0E-4) const {
            return source_->density(adjustedStrike(strike), discount, gap);
        }

      protected:

        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;

      private:

        Real adjustedStrike(Real strike) const;
        boost::shared_ptr<SmileSection> source_;
        Real adjustment_;
        Real f_;
    };
}

#endif
