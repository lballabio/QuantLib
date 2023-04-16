/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file treecapfloorengine.hpp
    \brief Numerical lattice engine for cap/floors
*/

#ifndef quantlib_pricers_tree_cap_floor_hpp
#define quantlib_pricers_tree_cap_floor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/pricingengines/latticeshortratemodelengine.hpp>

namespace QuantLib {

    //! Numerical lattice engine for cap/floors
    /*! \ingroup capfloorengines */
    class TreeCapFloorEngine
        : public LatticeShortRateModelEngine<CapFloor::arguments,
                                             CapFloor::results> {
      public:
        /*! \name Constructors
            \note the term structure is only needed when the short-rate
                  model cannot provide one itself.
        */
        //@{
        TreeCapFloorEngine(const std::shared_ptr<ShortRateModel>& model,
                           Size timeSteps,
                           Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>());
        TreeCapFloorEngine(const std::shared_ptr<ShortRateModel>& model,
                           const TimeGrid& timeGrid,
                           Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>());
        //@}
        void calculate() const override;

      private:
        Handle<YieldTermStructure> termStructure_;
    };

}


#endif

