
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

/*! \file couponbondoption.hpp
    \brief Coupon bond option pricer

    \fullpath
    ql/Pricers/%couponbondoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_coupon_bond_option_h
#define quantlib_pricers_coupon_bond_option_h

#include "ql/option.hpp"
#include "ql/InterestRateModelling/model.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        using InterestRateModelling::Model;

        //! Coupon Bond Option
        class CouponBondOption {
          public:
            CouponBondOption( Time maturity, const std::vector<Time>& times,
                const std::vector<double>& amounts, Option::Type type,
                double strike, const Handle<Model>& model);
            double value(Rate rate);
          private:
            class rStarFinder;
            friend class rStarFinder;

            Time maturity_;
            const std::vector<Time>& times_;
            const std::vector<double>& amounts_;
            Option::Type type_;
            double strike_;
            const Handle<Model>& model_;
        };

    }
}

#endif
