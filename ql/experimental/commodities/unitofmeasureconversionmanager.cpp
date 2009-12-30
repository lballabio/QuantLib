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

#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/experimental/commodities/petroleumunitsofmeasure.hpp>
#include <ql/errors.hpp>

using namespace std;

namespace QuantLib {

    namespace {

        bool matches(const UnitOfMeasureConversion& c1,
                     const UnitOfMeasureConversion& c2) {
            return c1.commodityType() == c2.commodityType() &&
                ((c1.source() == c2.source() && c1.target() == c2.target())
                 || (c1.source() == c2.target() && c1.target() == c2.source()));
        }

        bool matches(const UnitOfMeasureConversion& c,
                     const CommodityType& commodityType,
                     const UnitOfMeasure& source,
                     const UnitOfMeasure& target) {
            return c.commodityType() == commodityType &&
                ((c.source() == source && c.target() == target)
                 || (c.source() == target && c.target() == source));
        }

        bool matches(const UnitOfMeasureConversion& c,
                     const CommodityType& commodityType,
                     const UnitOfMeasure& source) {
            return c.commodityType() == commodityType &&
                (c.source() == source || c.target() == source);
        }

    }

    UnitOfMeasureConversionManager::UnitOfMeasureConversionManager()
    : Singleton<UnitOfMeasureConversionManager>() {
        addKnownConversionFactors();
    }

    void UnitOfMeasureConversionManager::add(const UnitOfMeasureConversion& c) {
        // not fast, but hopefully we won't have a lot of entries.
        for (list<UnitOfMeasureConversion>::iterator i = data_.begin();
             i != data_.end(); ++i) {
            if (matches(*i, c)) {
                data_.erase(i);
                break;
            }
        }

        data_.push_back(c);
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

    void UnitOfMeasureConversionManager::clear() {
        data_.clear();
        addKnownConversionFactors();
    }

    void UnitOfMeasureConversionManager::addKnownConversionFactors() {
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    MBUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    1000));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    GallonUnitOfMeasure(),
                                    42));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    GallonUnitOfMeasure(),
                                    MBUnitOfMeasure(),
                                    1000 * 42));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    LitreUnitOfMeasure(),
                                    GallonUnitOfMeasure(),
                                    3.78541));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    BarrelUnitOfMeasure(),
                                    LitreUnitOfMeasure(),
                                    158.987));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    KilolitreUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    6.28981));
        add(UnitOfMeasureConversion(NullCommodityType(),
                                    TokyoKilolitreUnitOfMeasure(),
                                    BarrelUnitOfMeasure(),
                                    6.28981));
    }

    UnitOfMeasureConversion UnitOfMeasureConversionManager::directLookup(
                                           const CommodityType& commodityType,
                                           const UnitOfMeasure& source,
                                           const UnitOfMeasure& target) const {

        for (list<UnitOfMeasureConversion>::const_iterator i = data_.begin();
             i != data_.end(); ++i) {
            if (matches(*i, commodityType, source, target)) {
                return *i;
            }
        }

        // Here, the code used to look for conversions with null
        // commodity type as a fall-back.  However, this would only
        // affect direct lookups and not other matches being tried in
        // the smart-lookup loop.  To implement the complete fall-back
        // strategy, we should either duplicate the loop (as we would
        // duplicate it here---smelly) or change the 'matches'
        // functions so that a null commodity type matches. However,
        // in the second case we would also have to take care that
        // conversions with a null type be at the end of the list so
        // that they don't supersede specific types. We'll have to
        // think a bit about this, so no fall-back for the time being.

        QL_FAIL("no direct conversion available from "
                << commodityType.code() << " " << source.code()
                << " to " << target.code());
    }

    UnitOfMeasureConversion UnitOfMeasureConversionManager::smartLookup(
                              const CommodityType& commodityType,
                              const UnitOfMeasure& source,
                              const UnitOfMeasure& target,
                              list<string> forbidden) const {

        try {
            return directLookup(commodityType,source,target);
        } catch (Error&) {
            ; // no direct conversion available; turn to smart lookup.
        }

        // The source unit is forbidden to subsequent lookups in order
        // to avoid cycles.
        forbidden.push_back(source.code());

        for (list<UnitOfMeasureConversion>::const_iterator i = data_.begin();
             i != data_.end(); ++i) {
            // we look for conversion data which involve our source unit...
            if (matches(*i, commodityType, source)) {
                const UnitOfMeasure& other =
                    source == i->source() ? i->target() : i->source();
                if (find(forbidden.begin(),forbidden.end(),
                         other.code()) == forbidden.end()) {
                    // if we can get to the target from here...
                    try {
                        UnitOfMeasureConversion tail =
                            smartLookup(commodityType,other,target);
                        // ..we're done.
                        return UnitOfMeasureConversion::chain(*i,tail);
                    } catch (Error&) {
                        // otherwise, we just discard this conversion.
                        ;
                    }
                }
            }
        }

        // if the loop completed, we have no way to return the
        // requested conversion.
        QL_FAIL("no conversion available for "
                << commodityType.code() << " from "
                << source.code() << " to " << target.code());
    }

}

