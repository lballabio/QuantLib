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

/*! \file gaussian1djamshidianswaptionengine.hpp
    \brief Swaption engine using Jamshidian's decomposition
*/

#ifndef quantlib_pricers_gaussian1d_jamshidian_swaption_hpp
#define quantlib_pricers_gaussian1d_jamshidian_swaption_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>

namespace QuantLib {

    //! Jamshidian swaption engine
    /*! \ingroup swaptionengines
    */
    class Gaussian1dJamshidianSwaptionEngine
        : public GenericModelEngine<Gaussian1dModel, Swaption::arguments,
                                    Swaption::results> {
      public:
        /*! \note the term structure is only needed when the short-rate
                  model cannot provide one itself.
        */
        Gaussian1dJamshidianSwaptionEngine(
            const ext::shared_ptr<Gaussian1dModel> &model)
            : GenericModelEngine<Gaussian1dModel, Swaption::arguments,
                                 Swaption::results>(model) {}
        void calculate() const override;

      private:
        class rStarFinder;
    };
}

#endif

