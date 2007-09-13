/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Katiuscia Manzoni

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

/*! \file optionletstripper.hpp
    \brief optionlet (cap/floor) volatility stripper
*/

#ifndef quantlib_optionletstripper_hpp
#define quantlib_optionletstripper_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatilities/interestrate/caplet/capletvolatilitiesstructures.hpp>

namespace QuantLib {
    class IborIndex;
    class YieldTermStructure;
    class CapVolatilitySurface;
    class SmileSection;

    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapFloorMatrix;

    class OptionletStripper : //public virtual Observer,
                           //public virtual Observable,
                           public LazyObject{
      public:
        OptionletStripper(const boost::shared_ptr<CapVolatilitySurface>& surface,
                       const boost::shared_ptr<IborIndex>& index);
        //! \name Cap Stripper interface
        //@{
        const Matrix& optionletPrices() const;
        const Matrix& optionletVolatilities() const;
        const Matrix& capfloorPrices() const;
        const Matrix& capfloorVolatilities() const;
        const std::vector<Period>& optionletTenors() const;
        const std::vector<Date>& optionletDates() const;
        const std::vector<Rate>& strikes() const;
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        //@}
      private:
        const boost::shared_ptr<CapVolatilitySurface> surface_;
        const boost::shared_ptr<IborIndex> index_;
        Size nStrikes_;
        std::vector<Period> optionletTenors_;
        Size nOptionletTenors_;
        mutable Matrix capfloorPrices_, optionletPrices_;
        mutable Matrix capfloorVols_, optionletVols_;
        mutable Matrix optionletStDevs_;
        mutable std::vector<Rate> atmOptionletRate;
        mutable std::vector<Date> optionletDates_;
        mutable std::vector<Time> optionletTimes_;
        std::vector<Period> capfloorLengths_;
        mutable CapFloorMatrix capfloors_;
    };

    inline const Matrix& OptionletStripper::optionletPrices() const {
        calculate();
        return optionletPrices_;
    }

    inline const Matrix& OptionletStripper::optionletVolatilities() const {
        calculate();
        return optionletVols_;
    }

    inline const Matrix& OptionletStripper::capfloorPrices() const {
        calculate();
        return capfloorPrices_;
    }

    inline const Matrix& OptionletStripper::capfloorVolatilities() const {
        calculate();
        return capfloorVols_;
    }

    inline
    const std::vector<Period>& OptionletStripper::optionletTenors() const {
        return optionletTenors_;
    }

    inline
    const std::vector<Date>& OptionletStripper::optionletDates() const {
        calculate();
        return optionletDates_;
    }

}

#endif
