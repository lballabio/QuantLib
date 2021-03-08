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

/*! \file fdmescrowedloginnervaluecalculator.hpp
    \brief inner value for a escrowed dividend model
*/

#ifndef quantlib_fdm_escrowed_log_inner_value_calculator_hpp
#define quantlib_fdm_escrowed_log_inner_value_calculator_hpp

#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/escroweddividendadjustment.hpp>

namespace QuantLib {

    class Payoff;
    class FdmMesher;

    class FdmEscrowedLogInnerValueCalculator: public FdmInnerValueCalculator {
      public:
        FdmEscrowedLogInnerValueCalculator(
            ext::shared_ptr<EscrowedDividendAdjustment> escrowedDividendAdj,
            ext::shared_ptr<Payoff> payoff,
            ext::shared_ptr<FdmMesher> mesher,
            Size direction);

        Real innerValue(const FdmLinearOpIterator& iter, Time t) override;
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override;

      private:
        const ext::shared_ptr<EscrowedDividendAdjustment> escrowedDividendAdj_;
        const ext::shared_ptr<Payoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Size direction_;
    };
}

#endif
