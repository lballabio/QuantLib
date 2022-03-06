/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2014, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Göttker-Schnetmann

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

/*! \file fdmhestonop.hpp
    \brief Heston linear operator
*/

#ifndef quantlib_fdm_heston_op_hpp
#define quantlib_fdm_heston_op_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class FdmHestonEquityPart {
      public:
        FdmHestonEquityPart(const ext::shared_ptr<FdmMesher>& mesher,
                            ext::shared_ptr<YieldTermStructure> rTS,
                            ext::shared_ptr<YieldTermStructure> qTS,
                            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
                            ext::shared_ptr<LocalVolTermStructure> leverageFct =
                                ext::shared_ptr<LocalVolTermStructure>());

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;
        const Array& getL() const { return L_; }

      protected:
        Disposable<Array> getLeverageFctSlice(Time t1, Time t2) const;

        Array varianceValues_, volatilityValues_, L_;
        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;

        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<YieldTermStructure> rTS_, qTS_;
        const ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        const ext::shared_ptr<LocalVolTermStructure> leverageFct_;
    };

    class FdmHestonVariancePart {
      public:
        FdmHestonVariancePart(const ext::shared_ptr<FdmMesher>& mesher,
                              ext::shared_ptr<YieldTermStructure> rTS,
                              Real mixedSigma,
                              Real kappa,
                              Real theta);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        const TripleBandLinearOp dyMap_;
        TripleBandLinearOp mapT_;

        const ext::shared_ptr<YieldTermStructure> rTS_;
    };


    class FdmHestonOp : public FdmLinearOpComposite {
      public:
        FdmHestonOp(const ext::shared_ptr<FdmMesher>& mesher,
                    const ext::shared_ptr<HestonProcess>& hestonProcess,
                    const ext::shared_ptr<FdmQuantoHelper>& quantoHelper =
                        ext::shared_ptr<FdmQuantoHelper>(),
                    const ext::shared_ptr<LocalVolTermStructure>& leverageFct =
                        ext::shared_ptr<LocalVolTermStructure>(),
                    Real mixingFactor = 1.0);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Disposable<Array> apply(const Array& r) const override;
        Disposable<Array> apply_mixed(const Array& r) const override;

        Disposable<Array> apply_direction(Size direction, const Array& r) const override;
        Disposable<Array> solve_splitting(Size direction, const Array& r, Real s) const override;
        Disposable<Array> preconditioner(const Array& r, Real s) const override;

        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const override;

      private:
        NinePointLinearOp correlationMap_;
        FdmHestonVariancePart dyMap_;
        FdmHestonEquityPart dxMap_;
    };
}

#endif


#ifndef id_f1d011f78f2809a15895d47df80ebd2e
#define id_f1d011f78f2809a15895d47df80ebd2e
inline bool test_f1d011f78f2809a15895d47df80ebd2e(int* i) { return i != 0; }
#endif
