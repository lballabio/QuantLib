
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

/*! \file blackswaptionengine.hpp
    \brief Black-formula swaption engine
*/

#ifndef quantlib_pricers_black_swaption_h
#define quantlib_pricers_black_swaption_h

#include <ql/Instruments/swaption.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>

namespace QuantLib {

    //! Black-formula swaption engine
    /*! \ingroup swaptionengines */
    class BlackSwaptionEngine : public GenericModelEngine<BlackModel, 
                                                          Swaption::arguments,
                                                          Swaption::results> {
      public:
        BlackSwaptionEngine(const boost::shared_ptr<BlackModel>& model)
        : GenericModelEngine<BlackModel, 
                             Swaption::arguments,
                             Swaption::results>(model) {}
        void calculate() const;
    };

    #if !defined(QL_DISABLE_DEPRECATED)
    //! \deprecated renamed to BlackSwaptionEngine
    typedef BlackSwaptionEngine BlackSwaption;
    #endif

}


#endif
