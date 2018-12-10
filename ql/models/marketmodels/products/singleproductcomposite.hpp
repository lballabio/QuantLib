/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#ifndef quantlib_single_product_composite_hpp
#define quantlib_single_product_composite_hpp

#include <ql/models/marketmodels/products/compositeproduct.hpp>

namespace QuantLib {

    //! Composition of one or more market-model products
    /*! Instances of this class build a single market-model product by
        composing two or more subproducts.

        \pre All subproducts must have the same rate times.
    */
    class SingleProductComposite : public MarketModelComposite {
      public:
        //! \name MarketModelMultiProduct interface
        //@{
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        bool nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated);
        #if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelMultiProduct> clone() const;
        #else
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        #endif
        //@}
    };

}


#endif
