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

#include <ql/experimental/commodities/unitofmeasureconversion.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::map<std::string, boost::shared_ptr<UnitOfMeasureConversion::Data> >
    UnitOfMeasureConversion::unitOfMeasureConversions_;

    UnitOfMeasureConversion::UnitOfMeasureConversion(
                                           const CommodityType& commodityType,
                                           const UnitOfMeasure& source,
                                           const UnitOfMeasure& target,
                                           Real conversionFactor,
                                           Type type) {
        std::string code = commodityType.name() + source.code() + target.code();
        std::map<std::string, boost::shared_ptr<UnitOfMeasureConversion::Data> >::const_iterator i = unitOfMeasureConversions_.find(code);
        if (i != unitOfMeasureConversions_.end())
            data_ = i->second;
        else {
            data_ = boost::shared_ptr<UnitOfMeasureConversion::Data>(
                   new UnitOfMeasureConversion::Data(commodityType,
                                                     source, target,
                                                     conversionFactor, type));
            unitOfMeasureConversions_[code] = data_;
        }
    }

    UnitOfMeasureConversion::UnitOfMeasureConversion(
                                            const UnitOfMeasureConversion& r1,
                                            const UnitOfMeasureConversion& r2) {
        std::string code = r1.code() + r2.code();
        std::map<std::string, boost::shared_ptr<UnitOfMeasureConversion::Data> >::const_iterator i = unitOfMeasureConversions_.find(code);
        if (i != unitOfMeasureConversions_.end())
            data_ = i->second;
        else {
            data_ = boost::shared_ptr<UnitOfMeasureConversion::Data>(
                                   new UnitOfMeasureConversion::Data(r1, r2));
            unitOfMeasureConversions_[code] = data_;
        }
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
        conversionFactorChain =
            std::make_pair(boost::shared_ptr<UnitOfMeasureConversion>(
                                             new UnitOfMeasureConversion(r1)),
                           boost::shared_ptr<UnitOfMeasureConversion>(
                                            new UnitOfMeasureConversion(r1)));
    }

    Quantity UnitOfMeasureConversion::convert(const Quantity& quantity) const {
        switch (data_->type) {
          case Direct:
            if (quantity.unitOfMeasure() == data_->source)
                return quantity*data_->conversionFactor;
            else if (quantity.unitOfMeasure() == data_->target)
                return quantity/data_->conversionFactor;
            else
                QL_FAIL("conversionFactor not applicable");
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
                QL_FAIL("exchange conversionFactor not applicable");
          default:
            QL_FAIL("unknown exchange-conversionFactor type");
        }
    }

    UnitOfMeasureConversion UnitOfMeasureConversion::chain(
                                            const UnitOfMeasureConversion& r1,
                                            const UnitOfMeasureConversion& r2) {
        UnitOfMeasureConversion result(r1, r2);
        if (r1.data_->source == r2.data_->source) {
            result.data_->source = r1.data_->target;
            result.data_->target = r2.data_->target;
        } else if (r1.data_->source == r2.data_->target) {
            result.data_->source = r1.data_->target;
            result.data_->target = r2.data_->source;
        } else if (r1.data_->target == r2.data_->source) {
            result.data_->source = r1.data_->source;
            result.data_->target = r2.data_->target;
        } else if (r1.data_->target == r2.data_->target) {
            result.data_->source = r1.data_->source;
            result.data_->target = r2.data_->source;
        } else {
            QL_FAIL("exchange conversionFactors not chainable");
        }
        return result;
    }

}
