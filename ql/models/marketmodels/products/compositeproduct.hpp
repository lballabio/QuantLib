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

#ifndef quantlib_market_model_composite_product_hpp
#define quantlib_market_model_composite_product_hpp

#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/utilities/clone.hpp>
#include <valarray>

namespace QuantLib {

    //! Composition of two or more market-model products
    /*! Instances of this class build a market-model product by
        composing one or more subproducts.

        \pre All subproducts must have the same rate times.
    */
    class MarketModelComposite : public MarketModelMultiProduct {
      public:
        MarketModelComposite() = default;
        //! \name MarketModelMultiProduct interface
        //@{
        const EvolutionDescription& evolution() const override;
        std::vector<Size> suggestedNumeraires() const override;
        std::vector<Time> possibleCashFlowTimes() const override;
        void reset() override;
        //@}
        //! \name Composite facilities
        //@{
        void add(const Clone<MarketModelMultiProduct>&,
                 Real multiplier = 1.0);
        void subtract(const Clone<MarketModelMultiProduct>&,
                      Real multiplier = 1.0);
        void finalize();
        Size size() const;
        const MarketModelMultiProduct& item(Size i) const;
        MarketModelMultiProduct& item(Size i);
        Real multiplier(Size i) const;
        //@}
      protected:
        // subproducts
        struct SubProduct {
            Clone<MarketModelMultiProduct> product;
            Real multiplier;
            std::vector<Size> numberOfCashflows;
            std::vector<std::vector<CashFlow> > cashflows;
            std::vector<Size> timeIndices;
            bool done;
        };
        std::vector<SubProduct> components_;
        typedef std::vector<SubProduct>::iterator iterator;
        typedef std::vector<SubProduct>::const_iterator const_iterator;
        // common evolution data
        std::vector<Time> rateTimes_;
        std::vector<Time> evolutionTimes_;
        EvolutionDescription evolution_;
        // working variables
        bool finalized_ = false;
        Size currentIndex_;
        std::vector<Time> cashflowTimes_;
        std::vector<std::vector<Time> > allEvolutionTimes_;
        std::vector<std::valarray<bool> > isInSubset_;
    };

}


#endif


#ifndef id_839c3386525e07874639c8becef9ee8a
#define id_839c3386525e07874639c8becef9ee8a
inline bool test_839c3386525e07874639c8becef9ee8a(int* i) { return i != 0; }
#endif
