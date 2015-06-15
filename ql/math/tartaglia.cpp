/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

#include <ql/math/tartaglia.hpp>
#include <iterator>

namespace QuantLib {

    std::vector<std::vector<BigNatural> > Tartaglia::coefficients_;

    const std::vector<BigNatural>& Tartaglia::get(Size order) {
        if (coefficients_.empty()) {
            // order zero mandatory for bootstrap
            coefficients_.push_back(std::vector<BigNatural>(1, 1));

            coefficients_.push_back(std::vector<BigNatural>(2, 1));
            coefficients_.push_back(std::vector<BigNatural>(3, 1));
            coefficients_[2][1] = 2;
            coefficients_.push_back(std::vector<BigNatural>(4, 1));
            coefficients_[3][1] = coefficients_[3][2] = 3;
        }
        while (coefficients_.size()<=order)
            nextOrder();
        return coefficients_[order];
    }

    void Tartaglia::nextOrder() {
        Size order = coefficients_.size();
        coefficients_.push_back(std::vector<BigNatural>(order+1));
        coefficients_[order][0] = coefficients_[order][order] = 1;
        for (Size i=1; i<order/2+1; ++i) {
            coefficients_[order][i] = coefficients_[order][order-i] =
                coefficients_[order-1][i-1] + coefficients_[order-1][i];
        }
    }

}
