/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_coterminal_to_forward_adapter_hpp
#define quantlib_coterminal_to_forward_adapter_hpp

#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>

namespace QuantLib {

    class CoterminalToForwardAdapter : public MarketModel {
      public:
          CoterminalToForwardAdapter(const boost::shared_ptr<MarketModel>& fwdModel);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const;
        const std::vector<Spread>& displacements() const;
        const EvolutionDescription& evolution() const;
        Size numberOfRates() const;
        Size numberOfFactors() const;
        Size numberOfSteps() const;
        const Matrix& pseudoRoot(Size i) const;
        const Matrix& covariance(Size i) const;
        const Matrix& totalCovariance(Size endIndex) const;
        //@}
      private:
        boost::shared_ptr<MarketModel> coterminalModel_;
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Matrix> pseudoRoots_, covariance_, totalCovariance_;
    };

    // inline

    inline const std::vector<Rate>& CoterminalToForwardAdapter::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>& CoterminalToForwardAdapter::displacements() const {
        return coterminalModel_->displacements();
    }

    inline const EvolutionDescription& CoterminalToForwardAdapter::evolution() const {
        return coterminalModel_->evolution();
    }

    inline Size CoterminalToForwardAdapter::numberOfRates() const {
        return coterminalModel_->numberOfRates();
    }

    inline Size CoterminalToForwardAdapter::numberOfFactors() const {
        return coterminalModel_->numberOfFactors();
    }

    inline Size CoterminalToForwardAdapter::numberOfSteps() const {
        return coterminalModel_->numberOfSteps();
    }

    inline const Matrix& CoterminalToForwardAdapter::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

    inline const Matrix& CoterminalToForwardAdapter::covariance(Size i) const {
        return covariance_[i];
    }

    inline const Matrix& CoterminalToForwardAdapter::totalCovariance(Size endIndex) const {
        return totalCovariance_[endIndex];
    }

}

#endif
