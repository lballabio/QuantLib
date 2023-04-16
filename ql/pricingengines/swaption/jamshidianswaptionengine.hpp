/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
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

/*! \file jamshidianswaptionengine.hpp
    \brief Swaption engine using Jamshidian's decomposition
    Concerning the start delay cf. http://ssrn.com/abstract=2246054
*/

#ifndef quantlib_pricers_jamshidian_swaption_hpp
#define quantlib_pricers_jamshidian_swaption_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <utility>

namespace QuantLib {

    //! Jamshidian swaption engine
    /*! \ingroup swaptionengines
        \warning The engine might assume that the exercise date equals the
                 start date of the passed swap unless the model provides
                 an implementation of the discountBondOption method with
                 start delay 
    */

    class JamshidianSwaptionEngine
        : public GenericModelEngine<OneFactorAffineModel,
                                    Swaption::arguments,
                                    Swaption::results > {
      public:
        /*! \note the term structure is only needed when the short-rate
                  model cannot provide one itself.
        */
        JamshidianSwaptionEngine(
            const std::shared_ptr<OneFactorAffineModel>& model,
            Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>())
        : GenericModelEngine<OneFactorAffineModel, Swaption::arguments, Swaption::results>(model),
          termStructure_(std::move(termStructure)) {
            registerWith(termStructure_);
        }
        void calculate() const override;

      private:
        Handle<YieldTermStructure> termStructure_;
        class rStarFinder;
    };

}


#endif

