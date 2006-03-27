/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discretizedconvertible.hpp
    \brief discretized convertible
*/

#ifndef quantlib_discretized_convertible_hpp
#define quantlib_discretized_convertible_hpp

#include <ql/discretizedasset.hpp>
#include <ql/Instruments/convertiblebond.hpp>

namespace QuantLib {

    class DiscretizedConvertible : public DiscretizedAsset {
      public:
        DiscretizedConvertible(const ConvertibleBond::option::arguments&);

        void reset(Size size);

        const Array& conversionProbability() const {
            return conversionProbability_;
        }
        Array& conversionProbability() { return conversionProbability_; }

        const Array& spreadAdjustedRate() const { return spreadAdjustedRate_; }
        Array& spreadAdjustedRate() { return spreadAdjustedRate_; }

        const Array& dividendValues() const { return dividendValues_; }
        Array& dividendValues() { return dividendValues_; }

        std::vector<Time> mandatoryTimes() const {
            return arguments_.stoppingTimes;
        }
      protected:
        void postAdjustValuesImpl();
        Array conversionProbability_, spreadAdjustedRate_, dividendValues_;

      private:
        void applyConvertibility();
        void applyCallability(Size);
        void addCoupon(Size);
        ConvertibleBond::option::arguments arguments_;
    };

}


#endif

