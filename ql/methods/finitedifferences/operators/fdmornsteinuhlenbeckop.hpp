/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmornsteinuhlenbeckop.hpp
    \brief Ornstein Uhlenbeck process
*/

#ifndef quantlib_fdm_ornstein_uhlenback_op_hpp
#define quantlib_fdm_ornstein_uhlenback_op_hpp

#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>


namespace QuantLib {

    class FdmMesher;
    class YieldTermStructure;
    class OrnsteinUhlenbeckProcess;

    class FdmOrnsteinUhlenbeckOp : public FdmLinearOpComposite {
      public:
        FdmOrnsteinUhlenbeckOp(const ext::shared_ptr<FdmMesher>& mesher,
                               ext::shared_ptr<OrnsteinUhlenbeckProcess> p,
                               ext::shared_ptr<YieldTermStructure> rTS,
                               Size direction = 0);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Array apply(const Array& r) const override;
        Array apply_mixed(const Array& r) const override;

        Array apply_direction(Size direction, const Array& r) const override;
        Array solve_splitting(Size direction, const Array& r, Real s) const override;
        Array preconditioner(const Array& r, Real s) const override;

        std::vector<SparseMatrix> toMatrixDecomp() const override;

      private:
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<OrnsteinUhlenbeckProcess> process_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
        const Size direction_;

        TripleBandLinearOp m_, mapX_;
    };

}
#endif
