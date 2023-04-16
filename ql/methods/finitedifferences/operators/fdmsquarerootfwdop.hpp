/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Klaus Spanderen
 Copyright (C) 2014 Johannes Göttker-Schnetmann

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

/*! \file fdmsquarerootfwdop.hpp
    \brief Square root linear operator for the Fokker-Planck forward equation
*/

#ifndef quantlib_fdm_square_root_fwd_op_hpp
#define quantlib_fdm_square_root_fwd_op_hpp

#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {
    class FdmMesher;
    class SquareRootProcess;
    class TripleBandLinearOp;
    class ModTripleBandLinearOp;

    class FdmSquareRootFwdOp : public FdmLinearOpComposite {
      public:
        enum TransformationType { Plain, Power, Log };

        FdmSquareRootFwdOp(
            const std::shared_ptr<FdmMesher>& mesher,
            Real kappa, Real theta, Real sigma,
            Size direction,
            TransformationType type = Plain);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Array apply(const Array& r) const override;
        Array apply_mixed(const Array& r) const override;
        Array apply_direction(Size direction, const Array& r) const override;
        Array solve_splitting(Size direction, const Array& r, Real s) const override;
        Array preconditioner(const Array& r, Real s) const override;

        std::vector<SparseMatrix> toMatrixDecomp() const override;

        Real lowerBoundaryFactor(TransformationType type = Plain) const;
        Real upperBoundaryFactor(TransformationType type = Plain) const;
        Real v(Size i) const;

      private:
        void setLowerBC(const std::shared_ptr<FdmMesher>& mesher);
        void setUpperBC(const std::shared_ptr<FdmMesher>& mesher);

        void getCoeff(Real& alpha, Real& beta,
                               Real& gamma, Size n) const;
        void getCoeffPlain(Real& alpha, Real& beta,
                               Real& gamma, Size n) const;
        void getCoeffPower(Real& alpha, Real& beta,
                               Real& gamma, Size n) const;
        void getCoeffLog(Real& alpha, Real& beta,
                               Real& gamma, Size n) const;

        Real f0Plain() const;
        Real f1Plain() const;
        Real f0Power() const;
        Real f1Power() const;
        Real f0Log() const;
        Real f1Log() const;

        Real h    (Size i) const;
        Real zetam(Size i) const;
        Real zeta (Size i) const;
        Real zetap(Size i) const;
        Real mu   (Size i) const;

        const Size direction_;
        const Real kappa_, theta_, sigma_;
        const TransformationType transform_;
        std::shared_ptr<ModTripleBandLinearOp> mapX_;
        Array v_;
    };
}

#endif
