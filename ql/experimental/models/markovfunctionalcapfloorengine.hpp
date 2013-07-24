/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file markovfunctionalcapfloorengine.hpp
    \brief
*/

#ifndef quantlib_pricers_markovFunctional_capfloor_hpp
#define quantlib_pricers_markovFunctional_capfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/experimental/models/markovfunctional.hpp>

namespace QuantLib {

    //! Markov functional cap/floor engine
    /*! \ingroup capfloorengines
        \warning The float leg is simplified in the sense that it is worth $P(t,T_0)-P(t,T_1)$ with $T_0$ and $T_1$ being the start date and end date of each caplet
        \warning Non zero spreads are not allowed
    */

    class MarkovFunctionalCapFloorEngine
        : public GenericModelEngine<MarkovFunctional,
                                    CapFloor::arguments,
                                    CapFloor::results > {
      public:
        MarkovFunctionalCapFloorEngine(
                         const boost::shared_ptr<MarkovFunctional>& model,
                         const int integrationPoints=64,
                         const Real stddevs=7.0,
                         const bool extrapolatePayoff=true,
                         const bool flatPayoffExtrapolation=false)
        : GenericModelEngine<MarkovFunctional,
                             CapFloor::arguments,
                             CapFloor::results>(model),
          integrationPoints_(integrationPoints), stddevs_(stddevs), extrapolatePayoff_(extrapolatePayoff), flatPayoffExtrapolation_(flatPayoffExtrapolation) { }
        void calculate() const;
      
    private:
        const int integrationPoints_;
        const Real stddevs_;
        const bool extrapolatePayoff_,flatPayoffExtrapolation_;

    };

}


#endif

