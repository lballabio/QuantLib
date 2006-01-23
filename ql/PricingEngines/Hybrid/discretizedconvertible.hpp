/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) Theo Boafo

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
#include <ql/Lattices/bsmlattice.hpp>
#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Instruments/convertiblebond.hpp>

namespace QuantLib {

    class DiscretizedConvertible : public DiscretizedAsset {
      public:
        DiscretizedConvertible(const ConvertibleBond::option::arguments& args)
        : arguments_(args) {}

        void reset(Size size);

        const Array& conversionProbability() const { return conversionProbability_; }
        Array& conversionProbability() { return conversionProbability_; }

        const Array& spreadAdjustRate() const { return spreadAdjustRate_; }
        Array& spreadAdjustRate() { return spreadAdjustRate_; }

        const Array& putValues() const { return putValues_; }
        Array& putValues() { return putValues_; }

        const Array& callValues() const { return callValues_; }
        Array& callValues() { return callValues_; }

        const Array& dividendValues() const { return dividendValues_; }
        Array& dividendValues() { return dividendValues_; }

        std::vector<Time> mandatoryTimes() const {
            return arguments_.stoppingTimes;
        }
      protected:
        void postAdjustValuesImpl();
        Array conversionProbability_, spreadAdjustRate_, putValues_ ,callValues_,
              dividendValues_;

      private:
        void applySpecificCondition();
        ConvertibleBond::option::arguments arguments_;
    };

}


#endif
