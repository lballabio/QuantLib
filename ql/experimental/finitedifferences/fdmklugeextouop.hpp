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

/*! \file fdmklugeextouop.hpp
    \brief Kluge process (power) plus Ornstein Uhlenbeck process (gas)
*/

#ifndef quantlib_fdm_kluge_ext_ou_op_hpp
#define quantlib_fdm_kluge_ext_ou_op_hpp

#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmboundaryconditionset.hpp>


namespace QuantLib {

    class FdmMesher;
    class YieldTermStructure;
    class KlugeExtOUProcess;
    class ExtOUWithJumpsProcess;
    class FdmExtOUJumpOp;
    class FdmExtendedOrnsteinUhlenbeckOp;
    class ExtendedOrnsteinUhlenbeckProcess;

    /*! This class describes a correlated Kluge - extended Ornstein-Uhlenbeck
        process governed by
        \f[
        \begin{array}{rcl}
            P_t &=& \exp(p_t + X_t + Y_t) \\
            dX_t &=& -\alpha X_tdt + \sigma_x dW_t^x \\
            dY_t &=& -\beta Y_{t-}dt + J_tdN_t \\
            \omega(J) &=& \eta e^{-\eta J} \\
            G_t &=& \exp(g_t + U_t) \\
            dU_t &=& -\kappa U_tdt + \sigma_udW_t^u \\
            \rho &=& \mathrm{corr} (dW_t^x, dW_t^u)
         \end{array}
         \f]
    */

    /*! References:
        Kluge, Timo L., 2008. Pricing Swing Options and other
        Electricity Derivatives, http://eprints.maths.ox.ac.uk/246/1/kluge.pdf

        http://spanderen.de/2011/06/13/vpp-pricing-i-stochastic-processes-partial-integro-differential-equation/
    */

    class FdmKlugeExtOUOp : public FdmLinearOpComposite {
      public:
        FdmKlugeExtOUOp(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<KlugeExtOUProcess>& klugeOUProcess,
            const ext::shared_ptr<YieldTermStructure>& rTS,
            const FdmBoundaryConditionSet& bcSet,
            Size integroIntegrationOrder);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Disposable<Array> apply(const Array& r) const override;
        Disposable<Array> apply_mixed(const Array& r) const override;

        Disposable<Array> apply_direction(Size direction, const Array& r) const override;
        Disposable<Array> solve_splitting(Size direction, const Array& r, Real s) const override;
        Disposable<Array> preconditioner(const Array& r, Real s) const override;

        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const override;

      private:

        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<ExtOUWithJumpsProcess> kluge_;
        const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> extOU_;

        const ext::shared_ptr<YieldTermStructure> rTS_;
        const FdmBoundaryConditionSet bcSet_;

        const ext::shared_ptr<FdmExtOUJumpOp> klugeOp_;
        const ext::shared_ptr<FdmExtendedOrnsteinUhlenbeckOp> ouOp_;

        const NinePointLinearOp corrMap_;
    };
}
#endif


#ifndef id_aa1b32188b6b3b3155909e37ea1690dc
#define id_aa1b32188b6b3b3155909e37ea1690dc
inline bool test_aa1b32188b6b3b3155909e37ea1690dc(const int* i) {
    return i != nullptr;
}
#endif
