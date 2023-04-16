/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2014 StatPro Italia srl

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

/*! \file region.hpp
    \brief Region, i.e. geographical area, specification
*/

#ifndef quantlib_region_hpp
#define quantlib_region_hpp

#include <ql/qldefines.hpp>
#include <memory>
#include <string>
#include <utility>

namespace QuantLib {

    //! Region class, used for inflation applicability.
    class Region {
      public:
        //! \name Inspectors
        //@{
        const std::string& name() const;
        const std::string& code() const;
        //@}
      protected:
        Region() = default;
        struct Data;
        std::shared_ptr<Data> data_;
    };

    struct Region::Data {
        std::string name;
        std::string code;
        Data(std::string name, std::string code) : name(std::move(name)), code(std::move(code)) {}
    };

    //! \relates Region
    bool operator==(const Region&, const Region&);

    //! \relates Region
    bool operator!=(const Region&, const Region&);


    //! Custom geographical/economic region
    /*! This class allows one to create an instance of a particular
        region without having to define and compile a corresponding
        class.
    */
    class CustomRegion : public Region {
      public:
        CustomRegion(const std::string& name,
                     const std::string& code);
    };


    //! Australia as geographical/economic region
    class AustraliaRegion : public Region {
      public:
        AustraliaRegion();
    };

    //! European Union as geographical/economic region
    class EURegion : public Region {
      public:
        EURegion();
    };

    //! France as geographical/economic region
    class FranceRegion : public Region {
      public:
        FranceRegion();
    };

    //! United Kingdom as geographical/economic region
    class UKRegion : public Region {
      public:
        UKRegion();
    };

    //! USA as geographical/economic region
    class USRegion : public Region {
    public:
        USRegion();
    };

    //! South Africa as geographical/economic region
    class ZARegion : public Region {
    public:
        ZARegion();
    };


    // inline definitions

    inline const std::string& Region::name() const {
        return data_->name;
    }

    inline const std::string& Region::code() const {
        return data_->code;
    }

    inline bool operator==(const Region& r1, const Region& r2) {
        return r1.name() == r2.name();
    }

    inline bool operator!=(const Region& r1, const Region& r2) {
        return !(r1.name() == r2.name());
    }

}

#endif
