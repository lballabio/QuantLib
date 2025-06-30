/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Paolo D'Elia

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

#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/errors.hpp>
#include <ql/utilities/variants.hpp>

namespace QuantLib {

    Handle<Quote> handleFromVariant(const std::variant<Real, Handle<Quote>>& value) {
        return std::visit(
            detail::variant_visitor{
                [](Real x) -> Handle<Quote> { return makeQuoteHandle(x); },
                [](const Handle<Quote>& x) { return x; }
            },
            value);
    }

}
