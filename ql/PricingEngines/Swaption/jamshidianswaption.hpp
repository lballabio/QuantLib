
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

/*! \file jamshidianswaption.hpp
    \brief Swaption pricer using Jamshidian's decomposition
*/

#ifndef quantlib_pricers_jamshidian_swaption_h
#define quantlib_pricers_jamshidian_swaption_h

#include <ql/PricingEngines/Swaption/swaptionpricer.hpp>
#include <ql/ShortRateModels/onefactormodel.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>

namespace QuantLib {

    //! Jamshidian swaption pricer
    class JamshidianSwaption 
        : public GenericModelEngine<OneFactorAffineModel,
                                    Swaption::arguments,
                                    Swaption::results > {
      public:
        JamshidianSwaption(const boost::shared_ptr<OneFactorAffineModel>& modl)
        : GenericModelEngine<OneFactorAffineModel,
                             Swaption::arguments,
                             Swaption::results >(modl) {} 
        void calculate() const;
      private:
        class rStarFinder;
        friend class rStarFinder;
    };

}


#endif
