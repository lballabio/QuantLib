
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

/*! \file analyticalcapfloor.hpp
    \brief Analytical pricer for caps/floors
*/

#ifndef quantlib_pricers_analytical_cap_floor_h
#define quantlib_pricers_analytical_cap_floor_h

#include <ql/PricingEngines/CapFloor/capfloorpricer.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>
#include <ql/ShortRateModels/model.hpp>

namespace QuantLib {

    //! Analytical pricer for cap/floor
    class AnalyticalCapFloor 
        : public GenericModelEngine<AffineModel, 
                                    CapFloor::arguments,
                                    CapFloor::results > {
      public:
        AnalyticalCapFloor(const boost::shared_ptr<AffineModel>& model) 
        : GenericModelEngine<AffineModel, 
                             CapFloor::arguments,
                             CapFloor::results >(model) 
        {}
        void calculate() const;
    };

}


#endif
