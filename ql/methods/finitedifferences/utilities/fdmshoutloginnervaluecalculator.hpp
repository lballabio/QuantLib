/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file fdmshoutloginnervaluecalculator.hpp
    \brief inner value for a shout option
*/

#ifndef quantlib_fdm_shout_log_inner_value_calculator_hpp
#define quantlib_fdm_shout_log_inner_value_calculator_hpp

#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class PlainVanillaPayoff;
    class GeneralizedBlackScholesProcess;

    class FdmShoutLogInnerValueCalculator: public FdmInnerValueCalculator {
      public:
        FdmShoutLogInnerValueCalculator(
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Time maturity,
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            const ext::shared_ptr<FdmMesher>& mesher,
            Size direction);

        Real innerValue(const FdmLinearOpIterator& iter, Time t) override;
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Time maturity_;
        const ext::shared_ptr<PlainVanillaPayoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Size direction_;
    };
}

#endif
