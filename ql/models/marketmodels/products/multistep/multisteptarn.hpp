/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Mark Joshi

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

#ifndef quantlib_multistep_tarn_hpp
#define quantlib_multistep_tarn_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>

namespace QuantLib {

    class MultiStepTarn : public MultiProductMultiStep {
      public:
        MultiStepTarn(const std::vector<Time>& rateTimes,
                         const std::vector<Real>& accruals,
                         const std::vector<Real>& accrualsFloating,                         
                         const std::vector<Time>& paymentTimes,
                         const std::vector<Time>& paymentTimesFloating,
                         Real totalCoupon,
                         const std::vector<Real>& strikes,
                         const std::vector<Real>& multipliers,
                         const std::vector<Real>& floatingSpreads);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated);
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        //@}
      private:
        std::vector<Real> accruals_;
        std::vector<Real> accrualsFloating_;
        std::vector<Time> paymentTimes_;
        std::vector<Time> paymentTimesFloating_;
        std::vector<Time> allPaymentTimes_;
        Real totalCoupon_;
        std::vector<Real> strikes_;
        std::vector<Real> multipliers_;
        std::vector<Real> floatingSpreads_;
        Size lastIndex_;
      
        // things that vary in a path
        Real couponPaid_;
        Size currentIndex_;
    };




}

#endif
