
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file treeswaption.hpp
    \brief Swaption calculated using a tree

    \fullpath
    ql/Pricers/%treeswaption.hpp
*/

// $Id$

#ifndef quantlib_pricers_european_swaption_h
#define quantlib_pricers_european_swaption_h

#include "ql/exercise.hpp"
#include "ql/option.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"

#include <vector>

namespace QuantLib {

    namespace Pricers {

        //! Swaption priced in a tree
        class TreeSwaption {
          public:
            TreeSwaption() {}
            TreeSwaption(
                bool payFixed,
                Exercise::Type exerciseType,
                const std::vector<Time>& maturities,
                Time start,
                const std::vector<Time>& payTimes,
                const std::vector<double>& coupons,
                double nominal,
                size_t timeSteps);
            void useModel(
                const Handle<InterestRateModelling::OneFactorModel>& model) {
                model_ = model;
            }
            void calculate();
            double value() const { return value_; }

          private:
            bool payFixed_;
            Exercise::Type exerciseType_;
            std::vector<Time> maturities_;
            Time start_;
            std::vector<Time> payTimes_;
            std::vector<double> coupons_;
            double nominal_;
            double value_;
            Lattices::TimeGrid timeGrid_;
            Handle<InterestRateModelling::OneFactorModel> model_;
        };

    }

}

#endif
