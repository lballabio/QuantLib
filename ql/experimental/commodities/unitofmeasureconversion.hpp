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

/*! \file energycommodity.hpp
    \brief Energy commodity
*/

#ifndef quantlib_unit_of_measure_conversion_hpp
#define quantlib_unit_of_measure_conversion_hpp

#include <ql/experimental/commodities/quantity.hpp>
#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <ql/utilities/null.hpp>
#include <utility>

namespace QuantLib {

    class UnitOfMeasureConversion {
      public:
        enum Type { Direct,  /*!< given directly by the user */
                    Derived  /*!< derived from conversion factors
                                  between other uoms */
        };
        //! \name Constructors
        //@{
        UnitOfMeasureConversion() = default;
        /*! the conversionFactor \f$ r \f$ is given with the
            convention that a unit of the source is worth \f$ r \f$
            units of the target.
        */
        UnitOfMeasureConversion(const CommodityType& commodityType,
                                const UnitOfMeasure& source,
                                const UnitOfMeasure& target,
                                Real conversionFactor);
        //@}

        //! \name Inspectors
        //@{
        //! the source UOM.
        const UnitOfMeasure& source() const;
        //! the target UOM.
        const UnitOfMeasure& target() const;
        //! the commodity type.
        const CommodityType& commodityType() const;
        //! the type
        Type type() const;
        //! the conversion factor
        Real conversionFactor() const;

        const std::string& code() const;
        //@}

        //! \name Utility methods
        //@{
        //! apply the conversion factor to a cash amount
        Quantity convert(const Quantity& quantity) const;
        //! chain two conversion factors
        static UnitOfMeasureConversion chain(const UnitOfMeasureConversion& r1,
                                             const UnitOfMeasureConversion& r2);
        //@}
      protected:
        UnitOfMeasureConversion(const UnitOfMeasureConversion& r1,
                                const UnitOfMeasureConversion& r2);

        struct Data;
        ext::shared_ptr<Data> data_;

        struct Data {
            CommodityType commodityType;
            UnitOfMeasure source, target;
            Real conversionFactor;
            Type type;
            std::string code;
            std::pair<ext::shared_ptr<UnitOfMeasureConversion>,
                      ext::shared_ptr<UnitOfMeasureConversion> >
            conversionFactorChain;

            Data(const CommodityType& commodityType,
                 const UnitOfMeasure& source,
                 const UnitOfMeasure& target,
                 Real conversionFactor,
                 Type type);

            Data(const UnitOfMeasureConversion& r1,
                 const UnitOfMeasureConversion& r2);
        };
    };

    // inline definitions

    inline const CommodityType& UnitOfMeasureConversion::commodityType() const {
        return data_->commodityType;
    }

    inline const UnitOfMeasure& UnitOfMeasureConversion::source() const {
        return data_->source;
    }

    inline const UnitOfMeasure& UnitOfMeasureConversion::target() const {
        return data_->target;
    }

    inline Real UnitOfMeasureConversion::conversionFactor() const {
        return data_->conversionFactor;
    }

    inline UnitOfMeasureConversion::Type UnitOfMeasureConversion::type() const {
        return data_->type;
    }

    inline const std::string& UnitOfMeasureConversion::code() const {
        return data_->code;
    }

}

#endif


#ifndef id_a1323c3805cf4ccdc88bd473c38e0297
#define id_a1323c3805cf4ccdc88bd473c38e0297
inline bool test_a1323c3805cf4ccdc88bd473c38e0297(const int* i) {
    return i != nullptr;
}
#endif
