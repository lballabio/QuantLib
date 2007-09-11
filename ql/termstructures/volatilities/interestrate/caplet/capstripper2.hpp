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

/*! \file capstripper2.hpp
    \brief caplet volatility stripper
*/

#ifndef quantlib_capstripper2_hpp
#define quantlib_capstripper2_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatilities/interestrate/caplet/capletvolatilitiesstructures.hpp>

namespace QuantLib {
    class IborIndex;
    class YieldTermStructure;
    class CapVolatilitySurface;
    class SmileSection;

    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;

    class CapletStripper : //public virtual Observer,
                           //public virtual Observable,
                           public LazyObject{
      public:
        CapletStripper(const boost::shared_ptr<CapVolatilitySurface>& surface,
                       const boost::shared_ptr<IborIndex>& index);
        //! \name Cap Stripper interface
        //@{
        const Matrix& capletPrices() const;
        const Matrix& capletVolatilities() const;
        const Matrix& capPrices() const;
        const Matrix& capVolatilities() const;
        const std::vector<Period>& optionTenors() const;
        const std::vector<Date>& optionDates() const;
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
        std::vector<Period> optionTenors_;
        Size nOptionTenors_;
        mutable Matrix capPrices_, capletPrices_;
        mutable Matrix capVols_, capletVols_;
        mutable Matrix capletStDevs_;
        mutable std::vector<Rate> atmCapletRate;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        mutable CapMatrix caps_;
    };

    inline const Matrix& CapletStripper::capletPrices() const {
        return capletPrices_;
    }

    inline const Matrix& CapletStripper::capletVolatilities() const {
        return capletVols_;
    }

    inline const Matrix& CapletStripper::capPrices() const {
        return capPrices_;
    }

    inline const Matrix& CapletStripper::capVolatilities() const {
        return capVols_;
    }

    inline const std::vector<Period>& CapletStripper::optionTenors() const {
        return optionTenors_;
    }

    inline const std::vector<Date>& CapletStripper::optionDates() const {
        return optionDates_;
    }

}

#endif
