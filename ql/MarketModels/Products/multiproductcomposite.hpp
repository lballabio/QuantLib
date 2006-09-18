/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_multiproduct_composite_hpp
#define quantlib_multiproduct_composite_hpp

#include <ql/MarketModels/marketmodelproduct.hpp>
#include <list>

namespace QuantLib {

    //! Composition of two or more market-model products
    /*! Instances of this class build a single market-model product by
        composing two or more subproducts.

        \pre All subproducts must have the same rate and evolution
             times.  If all subproducts have the same suggested
             numeraires, they are also used as the suggested ones for
             the composite product; otherwise, the terminal measure is
             used as a default.
    */
    class MultiProductComposite : public MarketModelMultiProduct {
      public:
        MultiProductComposite();
        //! \name MarketModelMultiProduct interface
        //@{
        EvolutionDescription suggestedEvolution() const;
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated);
        //@}
        //! \name Composite facilities
        //@{
        void add(const boost::shared_ptr<MarketModelMultiProduct>&,
                 Real multiplier = 1.0);
        void subtract(const boost::shared_ptr<MarketModelMultiProduct>&,
                      Real multiplier = 1.0);
        void finalize();
        //@}
      private:
        // subproducts
        struct SubProduct {
            boost::shared_ptr<MarketModelMultiProduct> product;
            Real multiplier;
            std::vector<Size> numberOfCashflows;
            std::vector<std::vector<CashFlow> > cashflows;
            std::vector<Size> timeIndices;
            Size offset;
            bool done;
        };
        std::list<SubProduct> components_;
        typedef std::list<SubProduct>::iterator iterator;
        typedef std::list<SubProduct>::const_iterator const_iterator;
        // common evolution data
        std::vector<Time> rateTimes_;
        std::vector<Time> evolutionTimes_;
        std::vector<Size> numeraires_;
        std::vector<std::pair<Size,Size> > relevanceRates_;
        // working variables
        bool finalized_;
        std::vector<Time> cashflowTimes_;
    };

}


#endif
