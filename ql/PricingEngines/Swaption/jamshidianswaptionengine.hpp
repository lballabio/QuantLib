
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

/*! \file jamshidianswaptionengine.hpp
    \brief Swaption engine using Jamshidian's decomposition
*/

#ifndef quantlib_pricers_jamshidian_swaption_h
#define quantlib_pricers_jamshidian_swaption_h

#include <ql/Instruments/swaption.hpp>
#include <ql/ShortRateModels/onefactormodel.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>

namespace QuantLib {

    //! Jamshidian swaption engine
    /*! \ingroup swaptionengines */
    class JamshidianSwaptionEngine 
        : public GenericModelEngine<OneFactorAffineModel,
                                    Swaption::arguments,
                                    Swaption::results > {
      public:
        JamshidianSwaptionEngine(
                         const boost::shared_ptr<OneFactorAffineModel>& model)
        : GenericModelEngine<OneFactorAffineModel,
                             Swaption::arguments,
                             Swaption::results>(model) {} 
        void calculate() const;
      private:
        class rStarFinder;
        friend class rStarFinder;
    };

    #if !defined(QL_DISABLE_DEPRECATED)
    //! \deprecated renamed to JamshidianSwaptionEngine
    typedef JamshidianSwaptionEngine JamshidianSwaption;
    #endif

}


#endif
