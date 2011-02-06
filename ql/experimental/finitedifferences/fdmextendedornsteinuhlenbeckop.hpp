/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmexpoujumpop.hpp
    \brief Ornstein Uhlenbeck process plus jumps (Kluge Model)
*/

#ifndef quantlib_fdm_extended_ornsteinuhlenback_op_hpp
#define quantlib_fdm_extended_ornsteinuhlenback_op_hpp

#include <ql/experimental/finitedifferences/firstderivativeop.hpp>
#include <ql/experimental/finitedifferences/triplebandlinearop.hpp>
#include <ql/experimental/finitedifferences/fdmlinearopcomposite.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>


namespace QuantLib {

    class FdmMesher;
    class YieldTermStructure;
    class ExtendedOrnsteinUhlenbeckProcess;

    class FdmExtendedOrnsteinUhlenbackOp : public FdmLinearOpComposite {
      public:
        FdmExtendedOrnsteinUhlenbackOp(
               const boost::shared_ptr<FdmMesher>& mesher,
               const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>& p,
               const boost::shared_ptr<YieldTermStructure>& rTS,
               const FdmBoundaryConditionSet& bcSet);

        Size size() const;
        void setTime(Time t1, Time t2);

        Disposable<Array> apply(const Array& r) const;
        Disposable<Array> apply_mixed(const Array& r) const;

        Disposable<Array> apply_direction(Size direction,
                                          const Array& r) const;
        Disposable<Array> solve_splitting(Size direction,
                                          const Array& r, Real s) const;
        Disposable<Array> preconditioner(const Array& r, Real s) const;

      private:
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> process_;
        const boost::shared_ptr<YieldTermStructure> rTS_;
        const FdmBoundaryConditionSet bcSet_;

        const Array x_;
        const FirstDerivativeOp dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapX_;

    };
}
#endif
