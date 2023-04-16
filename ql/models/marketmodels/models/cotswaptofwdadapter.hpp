/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

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

#ifndef quantlib_coterminal_to_forward_adapter_hpp
#define quantlib_coterminal_to_forward_adapter_hpp

#include <ql/models/marketmodels/marketmodel.hpp>

namespace QuantLib {

    class EvolutionDescription;

    class CotSwapToFwdAdapter : public MarketModel {
      public:
        CotSwapToFwdAdapter(
                       const std::shared_ptr<MarketModel>& coterminalModel);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const override;
        const std::vector<Spread>& displacements() const override;
        const EvolutionDescription& evolution() const override;
        Size numberOfRates() const override;
        Size numberOfFactors() const override;
        Size numberOfSteps() const override;
        const Matrix& pseudoRoot(Size i) const override;
        //@}
      private:
        std::shared_ptr<MarketModel> coterminalModel_;
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Matrix> pseudoRoots_;
    };


    class CotSwapToFwdAdapterFactory : public MarketModelFactory,
                                              public Observer {
      public:
        CotSwapToFwdAdapterFactory(
              const std::shared_ptr<MarketModelFactory>& coterminalFactory);
        std::shared_ptr<MarketModel> create(const EvolutionDescription&,
                                            Size numberOfFactors) const override;
        void update() override;

      private:
        std::shared_ptr<MarketModelFactory> coterminalFactory_;
    };


    // inline definitions

    inline const std::vector<Rate>&
    CotSwapToFwdAdapter::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>&
    CotSwapToFwdAdapter::displacements() const {
        return coterminalModel_->displacements();
    }

    inline const EvolutionDescription&
    CotSwapToFwdAdapter::evolution() const {
        return coterminalModel_->evolution();
    }

    inline Size CotSwapToFwdAdapter::numberOfRates() const {
        return coterminalModel_->numberOfRates();
    }

    inline Size CotSwapToFwdAdapter::numberOfFactors() const {
        return coterminalModel_->numberOfFactors();
    }

    inline Size CotSwapToFwdAdapter::numberOfSteps() const {
        return coterminalModel_->numberOfSteps();
    }

    inline const Matrix& CotSwapToFwdAdapter::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif
