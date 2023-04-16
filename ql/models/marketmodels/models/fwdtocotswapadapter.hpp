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

#ifndef quantlib_forward_to_coterminal_adapter_hpp
#define quantlib_forward_to_coterminal_adapter_hpp

#include <ql/models/marketmodels/marketmodel.hpp>

namespace QuantLib {

    class EvolutionDescription;

    class FwdToCotSwapAdapter : public MarketModel {
      public:
        FwdToCotSwapAdapter(
                          const std::shared_ptr<MarketModel>& forwardModel);
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
        std::shared_ptr<MarketModel> fwdModel_;
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Matrix> pseudoRoots_;
    };


    class FwdToCotSwapAdapterFactory : public MarketModelFactory,
                                              public Observer {
      public:
        FwdToCotSwapAdapterFactory(
              const std::shared_ptr<MarketModelFactory>& forwardFactory);
        std::shared_ptr<MarketModel> create(const EvolutionDescription&,
                                            Size numberOfFactors) const override;
        void update() override;

      private:
        std::shared_ptr<MarketModelFactory> forwardFactory_;
    };


    // inline definitions

    inline const std::vector<Rate>&
    FwdToCotSwapAdapter::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>&
    FwdToCotSwapAdapter::displacements() const {
        return fwdModel_->displacements();
    }

    inline const EvolutionDescription&
    FwdToCotSwapAdapter::evolution() const {
        return fwdModel_->evolution();
    }

    inline Size FwdToCotSwapAdapter::numberOfRates() const {
        return fwdModel_->numberOfRates();
    }

    inline Size FwdToCotSwapAdapter::numberOfFactors() const {
        return fwdModel_->numberOfFactors();
    }

    inline Size FwdToCotSwapAdapter::numberOfSteps() const {
        return fwdModel_->numberOfSteps();
    }

    inline const Matrix& FwdToCotSwapAdapter::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif
