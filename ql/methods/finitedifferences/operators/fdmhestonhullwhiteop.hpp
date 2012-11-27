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

/*! \file fdmhestonhullwhiteop.hpp
    \brief Heston Hull White linear operator
*/

#ifndef quantlib_fdm_heston_hull_white_op_hpp
#define quantlib_fdm_heston_hull_white_op_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/methods/finitedifferences/operators/fdmhullwhiteop.hpp>
#include <ql/methods/finitedifferences/operators/firstderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <ql/methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    class HullWhite;

    class FdmHestonHullWhiteEquityPart {
      public:
        FdmHestonHullWhiteEquityPart(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<HullWhite>& hwModel,
            const boost::shared_ptr<YieldTermStructure>& qTS);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        const Array x_;
        Array varianceValues_, volatilityValues_;
        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;

        const boost::shared_ptr<HullWhite> hwModel_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<YieldTermStructure> qTS_;
    };

    class FdmHestonHullWhiteOp : public FdmLinearOpComposite {
      public:
        FdmHestonHullWhiteOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<HestonProcess>& hestonProcess,
            const boost::shared_ptr<HullWhiteProcess>& hwProcess,
            Real equityShortRateCorrelation);

        Size size() const;
        void setTime(Time t1, Time t2);

        Disposable<Array> apply(const Array& r) const;
        Disposable<Array> apply_mixed(const Array& r) const;

        Disposable<Array> apply_direction(Size direction,
                                          const Array& r) const;
        Disposable<Array> solve_splitting(Size direction,
                                          const Array& r, Real s) const;
        Disposable<Array> preconditioner(const Array& r, Real s) const;

#if !defined(QL_NO_UBLAS_SUPPORT)
        Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const;
#endif
      private:
        const Real v0_, kappa_, theta_, sigma_, rho_;
        const boost::shared_ptr<HullWhite> hwModel_;

        NinePointLinearOp hestonCorrMap_;
        NinePointLinearOp equityIrCorrMap_;
        TripleBandLinearOp dyMap_;
        FdmHestonHullWhiteEquityPart dxMap_;
        FdmHullWhiteOp hullWhiteOp_;
    };
}

#endif
