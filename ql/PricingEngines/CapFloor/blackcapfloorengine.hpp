
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

/*! \file blackcapfloorengine.hpp
    \brief Black-formula cap/floor engine
*/

#ifndef quantlib_pricers_black_capfloor_h
#define quantlib_pricers_black_capfloor_h

#include <ql/Instruments/capfloor.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>

namespace QuantLib {

    //! Black-formula cap/floor engine
    /*! \ingroup capfloorengines */
    class BlackCapFloorEngine : public GenericModelEngine<BlackModel,
                                                          CapFloor::arguments,
                                                          CapFloor::results> {
      public:
        BlackCapFloorEngine(const boost::shared_ptr<BlackModel>& model)
        : GenericModelEngine<BlackModel, 
                             CapFloor::arguments,
                             CapFloor::results>(model) {}
        void calculate() const;
      private:
        Real capletValue(Time start, Rate forward,
                         Rate strike, Volatility vol) const;
        Real floorletValue(Time start, Rate forward,
                           Rate strike, Volatility vol) const;
    };

    /*
    #if !defined(QL_DISABLE_DEPRECATED)
    //! \deprecated renamed to BlackCapFloorEngine
    typedef BlackCapFloorEngine BlackCapFloor;
    #endif
    */

}


#endif
