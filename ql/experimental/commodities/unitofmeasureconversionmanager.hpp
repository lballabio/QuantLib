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

/*! \file unitofmeasureconversionmanager.hpp
    \brief Unit-of-measure conversion manager
*/

#ifndef quantlib_unit_of_measure_conversion_manager_hpp
#define quantlib_unit_of_measure_conversion_manager_hpp

#include <ql/experimental/commodities/unitofmeasureconversion.hpp>
#include <ql/patterns/singleton.hpp>
#include <ql/time/date.hpp>
#include <boost/functional/hash.hpp>
#include <list>
#include <map>

namespace QuantLib {

    class UnitOfMeasureConversionManager
        : public Singleton<UnitOfMeasureConversionManager> {
        friend class Singleton<UnitOfMeasureConversionManager>;

      public:
        UnitOfMeasureConversionManager();
        UnitOfMeasureConversion lookup(const CommodityType& commodityType,
                                       const UnitOfMeasure&,
                                       const UnitOfMeasure&,
                                       UnitOfMeasureConversion::Type type =
                                        UnitOfMeasureConversion::Derived) const;
        void add(const UnitOfMeasureConversion&) const;
        void clear() const;

        struct Entry {
            Entry() {}
            Entry(const UnitOfMeasureConversion& conversionFactor)
            : conversionFactor(conversionFactor) {}
            UnitOfMeasureConversion conversionFactor;
        };
      private:
        typedef BigInteger Key;
        boost::hash <std::string> string_hash;
        mutable std::map<Key, std::list<Entry> > data_;
        Key hash(const CommodityType& commodityType, const UnitOfMeasure& c1,
                 const UnitOfMeasure& c2) const;
        void addKnownConversionFactors() const;
        UnitOfMeasureConversion directLookup(const CommodityType& commodityType,
                                             const UnitOfMeasure& source,
                                             const UnitOfMeasure& target) const;
        UnitOfMeasureConversion smartLookup(
                                  const CommodityType& commodityType,
                                  const UnitOfMeasure& source,
                                  const UnitOfMeasure& target,
                                  const std::list<std::string>& forbiddenCodes
                                            = std::list<std::string>()) const;
        const UnitOfMeasureConversion* fetch(const CommodityType& commodityType,
                                             const UnitOfMeasure& source,
                                             const UnitOfMeasure& target) const;
    };

}


#endif
