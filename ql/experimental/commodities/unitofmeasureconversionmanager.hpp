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

/*! \file unitofmeasureconversionmanager.hpp
    \brief Unit-of-measure conversion manager
*/

#ifndef quantlib_unit_of_measure_conversion_manager_hpp
#define quantlib_unit_of_measure_conversion_manager_hpp

#include <ql/experimental/commodities/unitofmeasureconversion.hpp>
#include <ql/patterns/singleton.hpp>
#include <list>

namespace QuantLib {

    //! repository of conversion factors between units of measure
    /*! \test lookup of direct unit of measure conversion is tested. */
    class UnitOfMeasureConversionManager
        : public Singleton<UnitOfMeasureConversionManager> {
        friend class Singleton<UnitOfMeasureConversionManager>;

      public:
        UnitOfMeasureConversion lookup(
            const CommodityType& commodityType,
            const UnitOfMeasure&,
            const UnitOfMeasure&,
            UnitOfMeasureConversion::Type type =
                                    UnitOfMeasureConversion::Derived) const;
        void add(const UnitOfMeasureConversion&);
        void clear();

      private:
        std::list<UnitOfMeasureConversion> data_;
        UnitOfMeasureConversionManager();
        void addKnownConversionFactors();
        UnitOfMeasureConversion directLookup(const CommodityType& commodityType,
                                             const UnitOfMeasure& source,
                                             const UnitOfMeasure& target) const;
        UnitOfMeasureConversion smartLookup(const CommodityType& commodityType,
                                            const UnitOfMeasure& source,
                                            const UnitOfMeasure& target,
                                            std::list<std::string> forbidden =
                                                std::list<std::string>()) const;
    };

}

#endif


#ifndef id_65d84473682be212282a6e4c6b9cb801
#define id_65d84473682be212282a6e4c6b9cb801
inline bool test_65d84473682be212282a6e4c6b9cb801(const int* i) {
    return i != nullptr;
}
#endif
