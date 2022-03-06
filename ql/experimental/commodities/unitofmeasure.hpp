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

/*! \file unitofmeasure.hpp
    \brief Unit of measure
*/

#ifndef quantlib_unit_of_measure_hpp
#define quantlib_unit_of_measure_hpp

#include <ql/types.hpp>
#include <ql/math/rounding.hpp>
#include <ql/shared_ptr.hpp>
#include <string>
#include <map>
#include <iosfwd>

namespace QuantLib {

    //! %Unit of measure specification
    class UnitOfMeasure {
      public:
        enum Type { Mass, Volume, Energy, Quantity };
        //! default constructor
        /*! Instances built via this constructor have undefined
            behavior. Such instances can only act as placeholders
            and must be reassigned to a valid currency before being
            used.
        */
        UnitOfMeasure() = default;
        UnitOfMeasure(const std::string& name,
                      const std::string& code,
                      Type unitType);
        //! \name Inspectors
        //@{
        //! name, e.g, "Barrels"
        const std::string& name() const;
        //! code, e.g, "BBL", "MT"
        const std::string& code() const;
        //! unit type (mass, volume...)
        Type unitType() const;
        //@}
        //! \name Other information
        //@{
        //! is this a usable instance?
        bool empty() const;
        // rounding
        const Rounding& rounding() const;
        //! unit used for triangulation when required
        const UnitOfMeasure& triangulationUnitOfMeasure() const;
        //@}
      protected:
        struct Data;
        ext::shared_ptr<Data> data_;
      private:
        static std::map<std::string, ext::shared_ptr<UnitOfMeasure::Data> >
        unitsOfMeasure_;
    };

    struct UnitOfMeasure::Data {
        std::string name, code;
        UnitOfMeasure::Type unitType;
        UnitOfMeasure triangulationUnitOfMeasure;
        Rounding rounding;

        Data(std::string name,
             std::string code,
             UnitOfMeasure::Type unitType,
             UnitOfMeasure triangulationUnitOfMeasure = UnitOfMeasure(),
             const Rounding& rounding = Rounding(0));
    };


    /*! \relates UnitOfMeasure */
    bool operator==(const UnitOfMeasure&,
                    const UnitOfMeasure&);

    /*! \relates UnitOfMeasure */
    bool operator!=(const UnitOfMeasure&,
                    const UnitOfMeasure&);

    /*! \relates UnitOfMeasure */
    std::ostream& operator<<(std::ostream&,
                             const UnitOfMeasure&);


    // inline definitions

    inline const std::string& UnitOfMeasure::name() const {
        return data_->name;
    }

    inline const std::string& UnitOfMeasure::code() const {
        return data_->code;
    }

    inline UnitOfMeasure::Type UnitOfMeasure::unitType() const {
        return data_->unitType;
    }

    inline const Rounding& UnitOfMeasure::rounding() const {
        return data_->rounding;
    }

    inline bool UnitOfMeasure::empty() const {
        return !data_;
    }

    inline const UnitOfMeasure&
    UnitOfMeasure::triangulationUnitOfMeasure() const {
        return data_->triangulationUnitOfMeasure;
    }

    inline bool operator==(const UnitOfMeasure& c1, const UnitOfMeasure& c2) {
        return c1.code() == c2.code();
    }

    inline bool operator!=(const UnitOfMeasure& c1, const UnitOfMeasure& c2) {
        return !(c1 == c2);
    }

    class LotUnitOfMeasure : public UnitOfMeasure {
      public:
        LotUnitOfMeasure() {
            static ext::shared_ptr<Data> data(
                new Data("Lot", "Lot", UnitOfMeasure::Quantity));
            data_ = data;
        }
    };

}


#endif


#ifndef id_c5eb32ae606b8a84ff01cd2e345fc703
#define id_c5eb32ae606b8a84ff01cd2e345fc703
inline bool test_c5eb32ae606b8a84ff01cd2e345fc703(int* i) { return i != 0; }
#endif
