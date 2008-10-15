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

#include <ql/experimental/commodities/commodityindex.hpp>
#include <ql/experimental/commodities/commoditypricinghelpers.hpp>

namespace QuantLib {

    CommodityIndex::CommodityIndex(
                const std::string& indexName,
                const CommodityType& commodityType,
                const Currency& currency,
                const UnitOfMeasure& unitOfMeasure,
                const Calendar& calendar,
                Real lotQuantity,
                const boost::shared_ptr<CommodityCurve>& forwardCurve,
                const boost::shared_ptr<ExchangeContracts>& exchangeContracts,
                int nearbyOffset)
    : name_(indexName), commodityType_(commodityType),
      unitOfMeasure_(unitOfMeasure), currency_(currency), calendar_(calendar),
      lotQuantity_(lotQuantity), forwardCurve_(forwardCurve),
      forwardCurveUomConversionFactor_(1),
      exchangeContracts_(exchangeContracts), nearbyOffset_(nearbyOffset) {
        quotes_ = IndexManager::instance().getHistory(indexName);
        IndexManager::instance().setHistory(indexName, quotes_);
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));

        if (forwardCurve_ != 0)
            // registerWith(forwardCurve_);
            forwardCurveUomConversionFactor_ =
                CommodityPricingHelper::calculateUomConversionFactor(
                                                commodityType_,
                                                forwardCurve_->unitOfMeasure_,
                                                unitOfMeasure_);
    }

    std::ostream& operator<<(std::ostream& out, const CommodityIndex& index) {
        out << "[" << index.name_ << "] ("
            << index.currency_.code() << "/"
            << index.unitOfMeasure_.code() << ")";
        if (index.forwardCurve_ != 0)
            out << "; forward (" << (*index.forwardCurve_) << ")";
        return out;
    }

}
