/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <ostream>
#include <utility>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out, const UnitOfMeasure& c) {
        if (!c.empty())
            return out << c.code();
        else
            return out << "null unit of measure";
    }

    std::map<std::string,
             std::shared_ptr<UnitOfMeasure::Data> >
    UnitOfMeasure::unitsOfMeasure_;

    UnitOfMeasure::UnitOfMeasure(const std::string& name,
                                 const std::string& code,
                                 UnitOfMeasure::Type unitType) {
        std::map<std::string,
            std::shared_ptr<UnitOfMeasure::Data> >::const_iterator i =
            unitsOfMeasure_.find(name);
        if (i != unitsOfMeasure_.end()) {
            data_ = i->second;
        } else {
            data_ = std::make_shared<UnitOfMeasure::Data>(
                               name, code, unitType);
            unitsOfMeasure_[name] = data_;
        }
    }

    UnitOfMeasure::Data::Data(std::string name,
                              std::string code,
                              UnitOfMeasure::Type unitType,
                              UnitOfMeasure triangulationUnitOfMeasure,
                              const Rounding& rounding)
    : name(std::move(name)), code(std::move(code)), unitType(unitType),
      triangulationUnitOfMeasure(std::move(triangulationUnitOfMeasure)), rounding(rounding) {}
}

