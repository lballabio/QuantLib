/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

#include <ql/TermStructures/affinetermstructure.hpp>

namespace QuantLib {

    class AffineTermStructure::CalibrationFunction : public CostFunction {
      public:
        CalibrationFunction(
                   const boost::shared_ptr<ShortRateModel>& model,
                   const std::vector<boost::shared_ptr<RateHelper> >& helpers)
        : model_(model), instruments_(helpers) {}
        virtual ~CalibrationFunction() {}

        virtual Real value(const Array& params) const {
            model_->setParams(params);

            Real value = 0.0;
            for (Size i=0; i<instruments_.size(); i++) {
                Real diff = instruments_[i]->quoteError();
                value += diff*diff;
            }
            return value;
        }
        virtual Real finiteDifferenceEpsilon() const { return 1e-7; }
      private:
        boost::shared_ptr<ShortRateModel> model_;
        const std::vector<boost::shared_ptr<RateHelper> >& instruments_;
    };

    AffineTermStructure::AffineTermStructure(
                                  const Date& referenceDate,
                                  const boost::shared_ptr<AffineModel>& model,
                                  const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      model_(model) {}

    AffineTermStructure::AffineTermStructure(
               const Date& referenceDate,
               const boost::shared_ptr<AffineModel>& model,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const boost::shared_ptr<OptimizationMethod>& method,
               const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      model_(model), instruments_(instruments), method_(method) {
        for (Size i=0; i<instruments_.size(); i++)
            registerWith(instruments_[i]);
    }

    AffineTermStructure::AffineTermStructure(
                                  Integer settlementDays,
                                  const Calendar& calendar,
                                  const boost::shared_ptr<AffineModel>& model,
                                  const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      model_(model) {}

    AffineTermStructure::AffineTermStructure(
               Integer settlementDays,
               const Calendar& calendar,
               const boost::shared_ptr<AffineModel>& model,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const boost::shared_ptr<OptimizationMethod>& method,
               const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      model_(model), instruments_(instruments), method_(method) {
        for (Size i=0; i<instruments_.size(); i++)
            registerWith(instruments_[i]);
    }

    void AffineTermStructure::performCalculations() const {
        boost::shared_ptr<ShortRateModel> model =
            boost::dynamic_pointer_cast<ShortRateModel>(model_);
        CalibrationFunction f(model, instruments_);

        method_->setInitialValue(model->params());
        method_->endCriteria().setPositiveOptimization();
        Problem prob(f, *model->constraint(), *method_);
        prob.minimize();

        Array result(prob.minimumValue());
        model->setParams(result);
    }

}
