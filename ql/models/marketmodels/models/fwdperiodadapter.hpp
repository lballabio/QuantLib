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

#ifndef quantlib_forward_period_adapter_hpp
#define quantlib_forward_period_adapter_hpp

#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib {

 
    class FwdPeriodAdapter : public MarketModel {
      public:
        FwdPeriodAdapter(const ext::shared_ptr<MarketModel>& largeModel,
                         Size period,
                         Size offset,
                         std::vector<Spread> newDisplacements_);
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
        EvolutionDescription evolution_;
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Matrix> pseudoRoots_;
        std::vector<Spread> displacements_;
    };


   

    // inline definitions

    inline const std::vector<Rate>&
    FwdPeriodAdapter::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>&
    FwdPeriodAdapter::displacements() const {
        return displacements_;
    }

    inline const EvolutionDescription&
    FwdPeriodAdapter::evolution() const {
        return evolution_;
    }

    inline Size FwdPeriodAdapter::numberOfRates() const {
        return numberOfRates_;
    }

    inline Size FwdPeriodAdapter::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline Size FwdPeriodAdapter::numberOfSteps() const {
        return numberOfSteps_;
    }

    inline const Matrix& FwdPeriodAdapter::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif


#ifndef id_d3ac0467d94cef9eeea6bd76ac78ad92
#define id_d3ac0467d94cef9eeea6bd76ac78ad92
inline bool test_d3ac0467d94cef9eeea6bd76ac78ad92(const int* i) {
    return i != nullptr;
}
#endif
