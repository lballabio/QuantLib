
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file treeswaptionengine.hpp
    \brief Numerical lattice engine for swaptions
*/

#ifndef quantlib_pricers_tree_swaption_h
#define quantlib_pricers_tree_swaption_h

#include <ql/Instruments/swaption.hpp>
#include <ql/PricingEngines/latticeshortratemodelengine.hpp>

namespace QuantLib {

    //! Numerical lattice engine for swaptions
    /*! \ingroup swaptionengines

        \warning This engine is not guaranteed to work if the
                 underlying swap has a start date in the past. When
                 using this engine, prune the initial part of the swap
                 so that it starts at \f$ t \geq 0 \f$.

        \test calculations are checked against cached results
    */
    class TreeSwaptionEngine
    : public LatticeShortRateModelEngine<Swaption::arguments,
                                         Swaption::results> {
      public:
        TreeSwaptionEngine(const boost::shared_ptr<ShortRateModel>& model,
                           Size timeSteps);
        TreeSwaptionEngine(const boost::shared_ptr<ShortRateModel>& model,
                           const TimeGrid& timeGrid) ;
        void calculate() const;
    };

}


#endif
