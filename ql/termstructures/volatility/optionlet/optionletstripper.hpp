/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

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
*/

#ifndef quantlib_optionletstripper_hpp
#define quantlib_optionletstripper_hpp

#include <ql/termstructures/volatility/optionlet/optionletstripperbase.hpp>

namespace QuantLib {

    class IborIndex;

    class OptionletStripper : public OptionletStripperBase {
      public:
        OptionletStripper(const boost::shared_ptr<CapFloorTermVolSurface>&,
                          const boost::shared_ptr<IborIndex>& index);

        //! \name OptionletStripperBase interface
        //@{
        const std::vector<Rate>& optionletStrikes(Size i) const;
        const std::vector<Volatility>& optionletVolatilities(Size i) const;

        const std::vector<Date>& optionletDates() const;
        const std::vector<Time>& optionletTimes() const;

        const std::vector<Rate>& atmOptionletRate() const;

        boost::shared_ptr<CapFloorTermVolSurface> termVolSurface() const;
        //@}

        const std::vector<Period>& optionletTenors() const;
        boost::shared_ptr<IborIndex> index() const;
        const std::vector<Date>& optionletPaymentDates() const;
        const std::vector<Time>& optionletAccrualPeriods() const;

      protected:
        const boost::shared_ptr<CapFloorTermVolSurface> termVolSurface_;
        const boost::shared_ptr<IborIndex> index_; 
        Size nStrikes_; 
        Size nOptionletTenors_;

        mutable std::vector<std::vector<Rate> > optionletStrikes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;

        mutable std::vector<Time> optionletTimes_;
        mutable std::vector<Date> optionletDates_;
        std::vector<Period> optionletTenors_;
        mutable std::vector<Rate> atmOptionletRate_;
        mutable std::vector<Date> optionletPaymentDates_; 
        mutable std::vector<Time> optionletAccrualPeriods_;
        
        std::vector<Period> capFloorLengths_;

    };



}

#endif
