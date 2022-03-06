/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012 Klaus Spanderen

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

/*! \file fdmvppstartlimitstepcondition.hpp
    \brief VPP incl start limit step condition for FD models
*/

#ifndef quantlib_fdm_vpp_start_limit_step_condition_hpp
#define quantlib_fdm_vpp_start_limit_step_condition_hpp

#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class FdmVPPStartLimitStepCondition : public FdmVPPStepCondition {
      public:
        FdmVPPStartLimitStepCondition(
            const FdmVPPStepConditionParams& params,
            Size nStarts,
            const FdmVPPStepConditionMesher& mesh,
            const ext::shared_ptr<FdmInnerValueCalculator>& gasPrice,
            const ext::shared_ptr<FdmInnerValueCalculator>& sparkSpreadPrice);

        static Size nStates(Size tMinUp, Size tMinDown, Size nStarts);
        Real maxValue(const Array& states) const override;

      private:
        Disposable<Array> changeState(Real gasPrice, const Array& state, Time t) const override;

        const Size nStarts_;
    };
}

#endif


#ifndef id_24e0cb32e95bae426d15b5c59d689262
#define id_24e0cb32e95bae426d15b5c59d689262
inline bool test_24e0cb32e95bae426d15b5c59d689262(const int* i) {
    return i != nullptr;
}
#endif
