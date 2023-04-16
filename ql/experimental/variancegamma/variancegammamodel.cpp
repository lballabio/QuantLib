/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

#include <ql/experimental/variancegamma/variancegammamodel.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    VarianceGammaModel::VarianceGammaModel(
        const std::shared_ptr<VarianceGammaProcess> & process)
        : CalibratedModel(3), process_(process) {
            arguments_[0] = ConstantParameter(process->sigma(),
                PositiveConstraint());
            arguments_[1] = ConstantParameter(process->nu(),
                PositiveConstraint());
            arguments_[2] = ConstantParameter(process->theta(),
                NoConstraint());

            VarianceGammaModel::generateArguments();

            registerWith(process_->riskFreeRate());
            registerWith(process_->dividendYield());
            registerWith(process_->s0());
    }

    void VarianceGammaModel::generateArguments() {
        process_.reset(new VarianceGammaProcess(process_->s0(),
            process_->dividendYield(),
            process_->riskFreeRate(),
            sigma(), nu(), theta()));
    }

}

