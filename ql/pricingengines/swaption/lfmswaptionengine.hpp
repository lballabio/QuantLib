/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file lfmswaptionengine.hpp
    \brief libor forward model swaption engine based on black formula
*/

#ifndef quantlib_libor_forward_model_swaption_engine_hpp
#define quantlib_libor_forward_model_swaption_engine_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/legacy/libormarketmodels/liborforwardmodel.hpp>

namespace QuantLib {

    //! %Libor forward model swaption engine based on Black formula
    /*! \ingroup swaptionengines */
    class LfmSwaptionEngine : public GenericModelEngine<LiborForwardModel,
                                                        Swaption::arguments,
                                                        Swaption::results> {
      public:
        LfmSwaptionEngine(const boost::shared_ptr<LiborForwardModel>& model,
                          const Handle<YieldTermStructure>& discountCurve);
        void calculate() const;
      private:
        Handle<YieldTermStructure> discountCurve_;
    };

}


#endif
