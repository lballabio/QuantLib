
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

/*! \file blackcapfloor.hpp
    \brief CapFloor calculated using the Black formula
*/

#ifndef quantlib_pricers_black_capfloor_h
#define quantlib_pricers_black_capfloor_h

#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/PricingEngines/CapFloor/capfloorpricer.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>

namespace QuantLib {

    //! Cap/floor priced by means of the Black formula
    class BlackCapFloor : public GenericModelEngine<BlackModel, 
                                                    CapFloor::arguments,
                                                    CapFloor::results> {
      public:
        BlackCapFloor(const boost::shared_ptr<BlackModel>& model)
        : GenericModelEngine<BlackModel, 
                             CapFloor::arguments,
                             CapFloor::results>(model) {}
        void calculate() const;
      private:
        double capletValue(Time start, Rate forward,
                           Rate strike, double vol) const;
        double floorletValue(Time start, Rate forward,
                             Rate strike, double vol) const;
    };

}

#endif
