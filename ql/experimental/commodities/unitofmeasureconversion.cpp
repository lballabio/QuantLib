/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/experimental/commodities/unitofmeasureconversion.hpp>
#include <ql/errors.hpp>

using boost::shared_ptr;
using std::string;

namespace QuantLib {

    UnitOfMeasureConversion::UnitOfMeasureConversion(
                                           const CommodityType& commodityType,
                                           const UnitOfMeasure& source,
                                           const UnitOfMeasure& target,
                                           Real conversionFactor) {
        data_ = shared_ptr<UnitOfMeasureConversion::Data>(
              new UnitOfMeasureConversion::Data(commodityType, source, target,
                                                conversionFactor, Direct));
    }

    UnitOfMeasureConversion::UnitOfMeasureConversion(
                                            const UnitOfMeasureConversion& r1,
                                            const UnitOfMeasureConversion& r2) {
        data_ = shared_ptr<UnitOfMeasureConversion::Data>(
                                   new UnitOfMeasureConversion::Data(r1, r2));
    }

    UnitOfMeasureConversion::Data::Data(const CommodityType& commodityType,
                                        const UnitOfMeasure& source,
                                        const UnitOfMeasure& target,
                                        Real conversionFactor, Type type)
    : commodityType(commodityType), source(source), target(target),
      conversionFactor(conversionFactor), type(type) {
        code = commodityType.name() + source.code() + target.code();
    }

    UnitOfMeasureConversion::Data::Data(const UnitOfMeasureConversion& r1,
                                        const UnitOfMeasureConversion& r2) {
        conversionFactorChain = std::make_pair(
            shared_ptr<UnitOfMeasureConversion>(new UnitOfMeasureConversion(r1)),
            shared_ptr<UnitOfMeasureConversion>(new UnitOfMeasureConversion(r2)));
    }

    Quantity UnitOfMeasureConversion::convert(const Quantity& quantity) const {
        switch (data_->type) {
          case Direct:
            if (quantity.unitOfMeasure() == data_->source)
                return Quantity(quantity.commodityType(),
                                data_->target,
                                quantity.amount()*data_->conversionFactor);
            else if (quantity.unitOfMeasure() == data_->target)
                return Quantity(quantity.commodityType(),
                                data_->source,
                                quantity.amount()/data_->conversionFactor);
            else
                QL_FAIL("direct conversion not applicable");
          case Derived:
            if (quantity.unitOfMeasure()
                == data_->conversionFactorChain.first->source() ||
                quantity.unitOfMeasure()
                == data_->conversionFactorChain.first->target())
                return data_->conversionFactorChain.second->convert(
                       data_->conversionFactorChain.first->convert(quantity));
            else if (quantity.unitOfMeasure()
                     == data_->conversionFactorChain.second->source() ||
                     quantity.unitOfMeasure()
                     == data_->conversionFactorChain.second->target())
                return data_->conversionFactorChain.first->convert(
                      data_->conversionFactorChain.second->convert(quantity));
            else
                QL_FAIL("derived conversion factor not applicable");
          default:
            QL_FAIL("unknown conversion-factor type");
        }
    }

    UnitOfMeasureConversion UnitOfMeasureConversion::chain(
                                            const UnitOfMeasureConversion& r1,
                                            const UnitOfMeasureConversion& r2) {
        UnitOfMeasureConversion result(r1, r2);
        result.data_->type = Derived;
        if (r1.data_->source == r2.data_->source) {
            result.data_->source = r1.data_->target;
            result.data_->target = r2.data_->target;
            result.data_->conversionFactor =
                r2.data_->conversionFactor/r1.data_->conversionFactor;
        } else if (r1.data_->source == r2.data_->target) {
            result.data_->source = r1.data_->target;
            result.data_->target = r2.data_->source;
            result.data_->conversionFactor =
                1.0/(r1.data_->conversionFactor*r2.data_->conversionFactor);
        } else if (r1.data_->target == r2.data_->source) {
            result.data_->source = r1.data_->source;
            result.data_->target = r2.data_->target;
            result.data_->conversionFactor =
                r1.data_->conversionFactor*r2.data_->conversionFactor;
        } else if (r1.data_->target == r2.data_->target) {
            result.data_->source = r1.data_->source;
            result.data_->target = r2.data_->source;
            result.data_->conversionFactor =
                r1.data_->conversionFactor/r2.data_->conversionFactor;
        } else {
            QL_FAIL("conversion factors not chainable");
        }
        return result;
    }

}
