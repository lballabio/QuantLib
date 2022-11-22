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

/*! \file commodity.hpp
    \brief commodity type
*/

#ifndef quantlib_commodity_type_hpp
#define quantlib_commodity_type_hpp

#include <ql/qldefines.hpp>
#include <ql/shared_ptr.hpp>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>

namespace QuantLib {

    //! commodity type
    class CommodityType {
      public:
        //! default constructor
        /*! Instances built via this constructor have undefined
          behavior. Such instances can only act as placeholders
          and must be reassigned to a valid currency before being
          used.
        */
        CommodityType() = default;
        CommodityType(const std::string& code, const std::string& name);
        //! \name Inspectors
        //@{
        //! commodity code, e.g, "HO"
        const std::string& code() const;
        //! name, e.g, "Heating Oil"
        const std::string& name() const;
        // commodity code
        //@}
        //! \name Other information
        //@{
        //! is this a usable instance?
        bool empty() const;
        //@}

      protected:
        struct Data;
        ext::shared_ptr<Data> data_;

        struct Data {
            std::string name, code;

            Data(std::string name, std::string code)
            : name(std::move(name)), code(std::move(code)) {}
        };

        static std::map<std::string, ext::shared_ptr<Data> > commodityTypes_;
    };

    /*! \relates CommodityType */
    bool operator==(const CommodityType&,
                    const CommodityType&);

    /*! \relates CommodityType */
    bool operator!=(const CommodityType&,
                    const CommodityType&);

    /*! \relates CommodityType */
    std::ostream& operator<<(std::ostream&,
                             const CommodityType&);


    class NullCommodityType : public CommodityType {
      public:
        NullCommodityType() :
        CommodityType("<NULL>", "<NULL>") {}
    };


    inline const std::string& CommodityType::code() const {
        return data_->code;
    }

    inline const std::string& CommodityType::name() const {
        return data_->name;
    }

    inline bool CommodityType::empty() const {
        return !data_;
    }

    inline bool operator==(const CommodityType& c1, const CommodityType& c2) {
        return c1.code() == c2.code();
    }

    inline bool operator!=(const CommodityType& c1, const CommodityType& c2) {
        return !(c1 == c2);
    }

}


#endif
