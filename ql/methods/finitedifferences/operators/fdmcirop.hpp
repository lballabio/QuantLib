/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

/*! \file fdmcirop.hpp
    \brief CIR linear operator
*/

#ifndef quantlib_fdm_cir_op_hpp
#define quantlib_fdm_cir_op_hpp

#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/models/shortrate/onefactormodels/coxingersollross.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/coxingersollrossprocess.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class FdmCIREquityPart {
      public:
        FdmCIREquityPart(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<GeneralizedBlackScholesProcess> & bsProcess,
            Real strike);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;

        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<YieldTermStructure> qTS_;
        const Real strike_;
        const ext::shared_ptr<BlackVolTermStructure> sigma1_;
    };

    class FdmCIRRatesPart {
      public:
        FdmCIRRatesPart(
            const ext::shared_ptr<FdmMesher>& mesher,
            Real sigma, Real kappa, Real theta);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        const TripleBandLinearOp dyMap_;
        TripleBandLinearOp mapT_;
        const ext::shared_ptr<FdmMesher> mesher_;
    };

    class FdmCIRMixedPart {
      public:
        FdmCIRMixedPart(const ext::shared_ptr<FdmMesher>& mesher,
                        const ext::shared_ptr<CoxIngersollRossProcess>& cirProcess,
                        const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
                        Real rho,
                        Real strike);

        void setTime(Time t1, Time t2);
        const NinePointLinearOp& getMap() const;

      protected:
        const NinePointLinearOp dyMap_;
        NinePointLinearOp mapT_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<BlackVolTermStructure> sigma1_;
        const Real strike_;
    };


    class FdmCIROp : public FdmLinearOpComposite {
      public:
        FdmCIROp(const ext::shared_ptr<FdmMesher>& mesher,
                 const ext::shared_ptr<CoxIngersollRossProcess>& cirProcess,
                 const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
                 Real rho,
                 Real strike);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Disposable<Array> apply(const Array& r) const override;
        Disposable<Array> apply_mixed(const Array& r) const override;

        Disposable<Array> apply_direction(Size direction, const Array& r) const override;
        Disposable<Array> solve_splitting(Size direction, const Array& r, Real s) const override;
        Disposable<Array> preconditioner(const Array& r, Real s) const override;

        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const override;

      private:
        FdmCIREquityPart dxMap_;
        FdmCIRRatesPart dyMap_;
        FdmCIRMixedPart dzMap_;
    };
}

#endif


#ifndef id_f7ec6f6b6bac92fcb20ecaf94bc5e4ae
#define id_f7ec6f6b6bac92fcb20ecaf94bc5e4ae
inline bool test_f7ec6f6b6bac92fcb20ecaf94bc5e4ae(int* i) { return i != 0; }
#endif
