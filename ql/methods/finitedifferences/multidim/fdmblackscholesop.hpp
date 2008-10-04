/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file fdmblackscholesop.hpp
    \brief Black Scholes linear operator
*/

#ifndef quantlib_fdm_black_scholes_op_hpp
#define quantlib_fdm_black_scholes_op_hpp

#include <ql/payoff.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/multidim/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/multidim/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/multidim/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class FdmBlackScholesOp : public FdmLinearOpComposite {
      public:
        FdmBlackScholesOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            const boost::shared_ptr<Payoff>& payoff);

        Size size() const;
        void setTime(Time t1, Time t2);

        Disposable<Array> apply(const Array& r) const;
        Disposable<Array> apply_mixed(const Array& r) const;
        Disposable<Array> apply_direction(Size direction,
                                          const Array& r) const;
        Disposable<Array> solve_splitting(Size direction,
                                          const Array& r, Real s) const;

      private:
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<YieldTermStructure> rTS_, qTS_;
        const boost::shared_ptr<BlackVolTermStructure> volTS_;
        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;
        const Real strike_;
    };
}

#endif
