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

namespace QuantLib {

    class FdmHestonEquityPart {
      public:
        FdmHestonEquityPart(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const boost::shared_ptr<YieldTermStructure>& qTS,
            const boost::shared_ptr<FdmQuantoHelper>& quantoHelper);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        Array varianceValues_, volatilityValues_;
        const FirstDerivativeOp  dxMap_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;

        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<YieldTermStructure> rTS_, qTS_;
        const boost::shared_ptr<FdmQuantoHelper> quantoHelper_;
    };

    class FdmHestonVariancePart {
      public:
        FdmHestonVariancePart(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            Real sigma, Real kappa, Real theta);

        void setTime(Time t1, Time t2);
        const TripleBandLinearOp& getMap() const;

      protected:
        const TripleBandLinearOp dyMap_;
        TripleBandLinearOp mapT_;

        const boost::shared_ptr<YieldTermStructure> rTS_;
    };


    class FdmHestonOp : public FdmLinearOpComposite {
      public:
        FdmHestonOp(
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<HestonProcess>& hestonProcess,
            const boost::shared_ptr<FdmQuantoHelper>& quantoHelper
                                        = boost::shared_ptr<FdmQuantoHelper>());

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
        NinePointLinearOp correlationMap_;
        FdmHestonVariancePart dyMap_;
        FdmHestonEquityPart dxMap_;
    };
}

#endif
