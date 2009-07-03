/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/experimental/credit/defaulttype.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    DefaultType::DefaultType(AtomicDefault::Type defType,
                             Restructuring::Type restType)
    : defTypes_(defType), restrType_(restType) {
        // checks restruct and norestruct are never together.
        QL_REQUIRE((defType == AtomicDefault::Restructuring) != // xor
                   (restrType_ == Restructuring::NoRestructuring),
                   "Incoherent credit event type definition.");
    }

    bool operator==(const DefaultType& lhs, const DefaultType& rhs) {
        return (lhs.defaultType() == rhs.defaultType()) &&
            (lhs.restructuringType() == rhs.restructuringType());
    }

}

