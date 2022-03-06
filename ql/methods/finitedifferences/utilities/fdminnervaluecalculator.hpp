/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdminnervaluecalculator.hpp
    \brief layer of abstraction to calculate the inner value
*/

#ifndef quantlib_fdm_inner_value_calculator_hpp
#define quantlib_fdm_inner_value_calculator_hpp

#include <ql/types.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/math/functional.hpp>
#include <ql/functional.hpp>
#include <vector>


namespace QuantLib {

    class Payoff;
    class BasketPayoff;
    class FdmMesher;
    class FdmLinearOpIterator;


    class FdmInnerValueCalculator {
      public:
        virtual ~FdmInnerValueCalculator() = default;

        virtual Real innerValue(const FdmLinearOpIterator& iter, Time t) = 0;
        virtual Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) = 0;
    };


    class FdmCellAveragingInnerValue : public FdmInnerValueCalculator {
      public:
        FdmCellAveragingInnerValue(ext::shared_ptr<Payoff> payoff,
                                   ext::shared_ptr<FdmMesher> mesher,
                                   Size direction,
                                   ext::function<Real(Real)> gridMapping = identity<Real>());

        Real innerValue(const FdmLinearOpIterator& iter, Time) override;
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override;

      private:
        Real avgInnerValueCalc(const FdmLinearOpIterator& iter, Time t);

        const ext::shared_ptr<Payoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Size direction_;
        const ext::function<Real(Real)> gridMapping_;

        std::vector<Real> avgInnerValues_;
    };

    class FdmLogInnerValue : public FdmCellAveragingInnerValue {
      public:
        FdmLogInnerValue(const ext::shared_ptr<Payoff>& payoff,
                         const ext::shared_ptr<FdmMesher>& mesher,
                         Size direction);
    };

    class FdmLogBasketInnerValue : public FdmInnerValueCalculator {
      public:
        FdmLogBasketInnerValue(ext::shared_ptr<BasketPayoff> payoff,
                               ext::shared_ptr<FdmMesher> mesher);

        Real innerValue(const FdmLinearOpIterator& iter, Time) override;
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time) override;

      private:
        const ext::shared_ptr<BasketPayoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
    };

    class FdmZeroInnerValue : public FdmInnerValueCalculator {
      public:
        Real innerValue(const FdmLinearOpIterator&, Time) override { return 0.0; }
        Real avgInnerValue(const FdmLinearOpIterator&, Time) override { return 0.0; }
    };
}

#endif


#ifndef id_8b7d5ecd82fe6f88be098ebeac8cb53f
#define id_8b7d5ecd82fe6f88be098ebeac8cb53f
inline bool test_8b7d5ecd82fe6f88be098ebeac8cb53f(const int* i) {
    return i != nullptr;
}
#endif
