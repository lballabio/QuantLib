/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/default.hpp>

namespace QuantLib {

    DefaultEvent::DefaultEvent(const Date& date,
                               Real recoveryRate,
                               Seniority seniority,
                               Restructuring restructuring)
    : defaultDate_(date), recoveryRate_(recoveryRate),
      seniority_(seniority), restructuring_(restructuring) {}

    Date DefaultEvent::date() const {
        return defaultDate_;
    }

    Real DefaultEvent::recoveryRate() const {
        return recoveryRate_;
    }

    Seniority DefaultEvent::seniority() const {
        return seniority_;
    }

    Restructuring DefaultEvent::restructuring() const {
        return restructuring_;
    }

    void DefaultEvent::accept(AcyclicVisitor& v) {
        Visitor<DefaultEvent>* v1 =
            dynamic_cast<Visitor<DefaultEvent>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Event::accept(v);
    }

}

