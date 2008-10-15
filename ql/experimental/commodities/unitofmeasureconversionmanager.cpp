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

#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/experimental/commodities/petroleumunitsofmeasure.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        struct valid_at
            : public std::unary_function<UnitOfMeasureConversionManager::Entry,
                                         bool> {
            const CommodityType& commodityType;
            const UnitOfMeasure& source, target;
            valid_at(const CommodityType& commodityType,
                     const UnitOfMeasure& source, const UnitOfMeasure& target) :
            commodityType(commodityType), source(source), target(target) {}
            bool operator()(const UnitOfMeasureConversionManager::Entry& e) {
                return commodityType == e.conversionFactor.commodityType() &&
                       source == e.conversionFactor.source() &&
                       target == e.conversionFactor.target();
            }
        };

    }

    UnitOfMeasureConversionManager::UnitOfMeasureConversionManager()
    : Singleton<UnitOfMeasureConversionManager>() {
        addKnownConversionFactors();
    }

    void UnitOfMeasureConversionManager::add(
                      const UnitOfMeasureConversion& conversionFactor) const {
        Key k = hash(conversionFactor.commodityType(),
                     conversionFactor.source(),
                     conversionFactor.target());
        data_[k].push_front(Entry(conversionFactor));
    }

    UnitOfMeasureConversion UnitOfMeasureConversionManager::lookup(
                                   const CommodityType& commodityType,
                                   const UnitOfMeasure& source,
                                   const UnitOfMeasure& target,
                                   UnitOfMeasureConversion::Type type) const {
        if (type == UnitOfMeasureConversion::Direct) {
            return directLookup(commodityType,source,target);
        } else if (!source.triangulationUnitOfMeasure().empty()) {
            const UnitOfMeasure& link = source.triangulationUnitOfMeasure();
            if (link == target)
                return directLookup(commodityType,source,link);
            else
                return UnitOfMeasureConversion::chain(
                                      directLookup(commodityType,source,link),
                                      lookup(commodityType,link,target));
        } else if (!target.triangulationUnitOfMeasure().empty()) {
            const UnitOfMeasure& link = target.triangulationUnitOfMeasure();
            if (source == link)
                return directLookup(commodityType,link,target);
            else
                return UnitOfMeasureConversion::chain(
                                     lookup(commodityType,source,link),
                                     directLookup(commodityType,link,target));
        } else {
            return smartLookup(commodityType,source,target);
        }
    }

    void UnitOfMeasureConversionManager::clear() const {
        data_.clear();
        addKnownConversionFactors();
    }

    UnitOfMeasureConversionManager::Key
    UnitOfMeasureConversionManager::hash(const CommodityType& commodityType,
                                         const UnitOfMeasure& c1,
                                         const UnitOfMeasure& c2) const {
        return Key(string_hash(commodityType.code()) +
                   (string_hash(c1.code()) << 2)
                   + (string_hash(c2.code()) << 4));
    }

    void UnitOfMeasureConversionManager::addKnownConversionFactors() const {
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    MBUnitOfMeasure(),
                                    1000));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    MBUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    (Real)1 / 1000));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    GallonUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    42));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    GallonUnitOfMeasure(),
                                    (Real)1 / 42));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    GallonUnitOfMeasure(),
                                    MBUnitOfMeasure(),
                                    (Real)1 / (1000 * 42)));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    MBUnitOfMeasure(),
                                    GallonUnitOfMeasure(),
                                    (1000 * 42)));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    GallonUnitOfMeasure(),
                                    LitreUnitOfMeasure(),
                                    3.78541));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    LitreUnitOfMeasure(),
                                    GallonUnitOfMeasure(),
                                    (Real)1 / 3.78541));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    LitreUnitOfMeasure(),
                                    (Real)1 / 158.987));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    LitreUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    158.987));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    KilolitreUnitOfMeasure(),
                                    6.28981));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    KilolitreUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    (Real)1 / 6.28981));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    TokyoKilolitreUnitOfMeasure(),
                                    6.28981));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    TokyoKilolitreUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    (Real)1 / 6.28981));
    }

    UnitOfMeasureConversion UnitOfMeasureConversionManager::directLookup(
                                           const CommodityType& commodityType,
                                           const UnitOfMeasure& source,
                                           const UnitOfMeasure& target) const {
        if (const UnitOfMeasureConversion* conversionFactor =
            fetch(commodityType,source,target))
            return *conversionFactor;
        else
            QL_FAIL("no direct conversion available from "
                    << commodityType.code() << " " << source.code()
                    << " to " << target.code());
    }

    UnitOfMeasureConversion UnitOfMeasureConversionManager::smartLookup(
                              const CommodityType& commodityType,
                              const UnitOfMeasure& source,
                              const UnitOfMeasure& target,
                              const std::list<std::string>& forbidden) const {
        // direct exchange conversionFactors are preferred.
        if (const UnitOfMeasureConversion* direct =
            fetch(commodityType,source,target))
            return *direct;

#if FIXDME
        // if none is found, turn to smart lookup. The source currency
        // is forbidden to subsequent lookups in order to avoid cycles.
        forbidden.push_back(source.code());
        for (std::map<Key, std::list<Entry> >::const_iterator i = data_.begin();
             i != data_.end(); ++i) {
            // we look for exchange-conversionFactor data which
            // involve our source currency...
            if (!(i->second.empty())) {
                const Entry& e = i->second.front();
                const UnitOfMeasure& other =
                    source == e.conversionFactor.source() ?
                    e.conversionFactor.target() : e.conversionFactor.source();
                if (std::find(forbidden.begin(),forbidden.end(),
                              other.code()) == forbidden.end()) {
                    if (const UnitOfMeasureConversion* head =
                        fetch(commodityType,source,other)) {
                        // if we can get to the target from here...
                        try {
                            UnitOfMeasureConversion tail =
                                smartLookup(commodityType,other,target);
                            // ..we're done.
                            return UnitOfMeasureConversion::chain(*head,tail);
                        } catch (Error&) {
                            // otherwise, we just discard this conversionFactor.
                            ;
                        }
                    }
                }
            }
        }
#endif
        // if the loop completed, we have no way to return the
        // requested conversionFactor.
        QL_FAIL("no conversion available for "
                << commodityType.code() << " from "
                << source.code() << " to " << target.code());
    }

    const UnitOfMeasureConversion* UnitOfMeasureConversionManager::fetch(
                                           const CommodityType& commodityType,
                                           const UnitOfMeasure& source,
                                           const UnitOfMeasure& target) const {
        const std::list<Entry>& conversionFactors =
            data_[hash(commodityType,source,target)];
        std::list<Entry>::const_iterator i =
            std::find_if(conversionFactors.begin(), conversionFactors.end(),
                         valid_at(commodityType, source, target));
        if (i != conversionFactors.end())
            return &(i->conversionFactor);

        // if not found try to find one with a null commodity type
        NullCommodityType nullCommodityType;
        const std::list<Entry>& nullConversionFactors =
            data_[hash(nullCommodityType,source,target)];
        i = std::find_if(nullConversionFactors.begin(),
                         nullConversionFactors.end(),
                         valid_at(nullCommodityType, source, target));
        if (i != nullConversionFactors.end())
            return &(i->conversionFactor);
        return 0;
    }

}

