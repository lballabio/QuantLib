/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005, 2007 StatPro Italia srl

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

/*! \file treeswapengine.hpp
    \brief Numerical lattice engine for swaps
*/

#ifndef quantlib_tree_swap_engine_hpp
#define quantlib_tree_swap_engine_hpp

#include <ql/instruments/vanillaswap.hpp>
#include <ql/pricingengines/latticeshortratemodelengine.hpp>

namespace QuantLib {

    //! Numerical lattice engine for simple swaps
    /*! \test calculations are checked against known good results
    */
    class TreeVanillaSwapEngine
    : public LatticeShortRateModelEngine<VanillaSwap::arguments,
                                         VanillaSwap::results> {
      public:
        /*! \name Constructors
            \note the term structure is only needed when the short-rate
                  model cannot provide one itself.
        */
        //@{
        TreeVanillaSwapEngine(const boost::shared_ptr<ShortRateModel>&,
                              Size timeSteps,
                              const Handle<YieldTermStructure>& termStructure =
                                                 Handle<YieldTermStructure>());
        TreeVanillaSwapEngine(const boost::shared_ptr<ShortRateModel>&,
                              const TimeGrid& timeGrid,
                              const Handle<YieldTermStructure>& termStructure =
                                                 Handle<YieldTermStructure>());
        //@}
        void calculate() const;
      private:
        Handle<YieldTermStructure> termStructure_;
    };

}


#endif
