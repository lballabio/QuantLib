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

/*! \file triplebandlinearop.cpp
    \brief general triple band linear operator
*/

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/methods/finitedifferences/multidim/triplebandlinearop.hpp>

namespace QuantLib {

    TripleBandLinearOp::TripleBandLinearOp(
        Size direction,
        const boost::shared_ptr<FdmMesher>& mesher)
    : direction_(direction),
      i0_   (new Size[mesher->layout()->size()]),
      i2_   (new Size[mesher->layout()->size()]),
      lower_(new Real[mesher->layout()->size()]),
      diag_ (new Real[mesher->layout()->size()]),
      upper_(new Real[mesher->layout()->size()]),
      mesher_(mesher) {

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();

        for (FdmLinearOpIterator iter = layout->begin(); iter!=endIter; ++iter) {
            const Size i = iter.index();
            i0_[i] = layout->neighbourhood(iter, direction, -1);
            i2_[i] = layout->neighbourhood(iter, direction,  1);
        }
    }
        
    TripleBandLinearOp::TripleBandLinearOp(const TripleBandLinearOp& m)    
    : direction_(m.direction_),
      i0_   (new Size[m.mesher_->layout()->size()]),
      i2_   (new Size[m.mesher_->layout()->size()]),
      lower_(new Real[m.mesher_->layout()->size()]),
      diag_ (new Real[m.mesher_->layout()->size()]),
      upper_(new Real[m.mesher_->layout()->size()]),
      mesher_(m.mesher_) {
        const Size len = m.mesher_->layout()->size();
        std::copy(m.i0_.get(), m.i0_.get() + len, i0_.get());
        std::copy(m.i2_.get(), m.i2_.get() + len, i2_.get());
        std::copy(m.lower_.get(), m.lower_.get() + len, lower_.get());
        std::copy(m.diag_.get(),  m.diag_.get() + len,  diag_.get());
        std::copy(m.upper_.get(), m.upper_.get() + len, upper_.get());
    }
      

    TripleBandLinearOp::TripleBandLinearOp(
        const Disposable<TripleBandLinearOp>& from) {
        swap(const_cast<Disposable<TripleBandLinearOp>&>(from));
    }

    TripleBandLinearOp& TripleBandLinearOp::operator=(
        const TripleBandLinearOp& m) {
        TripleBandLinearOp tmp(m);
        swap(tmp);
        return *this;
    }
    
    TripleBandLinearOp& TripleBandLinearOp::operator=(
        const Disposable<TripleBandLinearOp>& m) {
        swap(const_cast<Disposable<TripleBandLinearOp>&>(m));
        return *this;
    }
    
    void TripleBandLinearOp::swap(TripleBandLinearOp& m) {
        std::swap(mesher_, m.mesher_);
        std::swap(direction_, m.direction_);

        i0_.swap(m.i0_); i2_.swap(m.i2_);
        lower_.swap(m.lower_); diag_.swap(m.diag_); upper_.swap(m.upper_);
    }

    void TripleBandLinearOp::axpyb(const Array& a, 
                                   const TripleBandLinearOp& x, 
                                   const TripleBandLinearOp& y,
                                   const Array& b) {
        const Size size = mesher_->layout()->size();
        
        Real *diag(diag_.get());
        Real *lower(lower_.get());
        Real *upper(upper_.get());
        
        const Real *y_diag (y.diag_.get());
        const Real *y_lower(y.lower_.get());
        const Real *y_upper(y.upper_.get());
        
        if (a.empty()) {
            if (b.empty()) {
                for (Size i=0; i < size; ++i) {
                    diag[i]  = y_diag[i];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }                
            }
            else {
                const Real *bptr(b.begin());
                const Size binc = (b.size() > 1) ? 1 : 0;
                
                for (Size i=0; i < size; ++i) {
                    diag[i]  = y_diag[i] + bptr[i*binc];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }
            }
        }
        else if (b.empty()) {
            const Real *aptr(a.begin());
            const Size ainc = (a.size() > 1) ? 1 : 0;

            const Real *x_diag (x.diag_.get());
            const Real *x_lower(x.lower_.get());
            const Real *x_upper(x.upper_.get());

            for (Size i=0; i < size; ++i) {
                const Real s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }
        }
        else {
            const Real *bptr(b.begin());
            const Size binc = (b.size() > 1) ? 1 : 0;
            
            const Real *aptr(a.begin());
            const Size ainc = (a.size() > 1) ? 1 : 0;

            const Real *x_diag (x.diag_.get());
            const Real *x_lower(x.lower_.get());
            const Real *x_upper(x.upper_.get());

            for (Size i=0; i < size; ++i) {
                const Real s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i] + bptr[i*binc];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }            
        }
    }

    Disposable<TripleBandLinearOp> 
        TripleBandLinearOp::add(const TripleBandLinearOp& m) const {

        TripleBandLinearOp retVal(direction_, mesher_);
        const Size size = mesher_->layout()->size();
        for (Size i=0; i < size; ++i) {
            retVal.lower_[i]= lower_[i] + m.lower_[i]; 
            retVal.diag_[i] = diag_[i]  + m.diag_[i]; 
            retVal.upper_[i]= upper_[i] + m.upper_[i];
        }

        return retVal;
    }


    Disposable<TripleBandLinearOp> TripleBandLinearOp::mult(const Array& u) 
        const {
        
        TripleBandLinearOp retVal(direction_, mesher_);

        const Size size = mesher_->layout()->size();
        for (Size i=0; i < size; ++i) {
            const Real s = u[i];
            retVal.lower_[i]= lower_[i]*s; 
            retVal.diag_[i] = diag_[i]*s; 
            retVal.upper_[i]= upper_[i]*s;
        }

        return retVal;
    }

    Disposable<TripleBandLinearOp> TripleBandLinearOp::add(const Array& u) 
        const {
        
        TripleBandLinearOp retVal(direction_, mesher_);

        const Size size = mesher_->layout()->size();
        for (Size i=0; i < size; ++i) {
            retVal.lower_[i]= lower_[i]; 
            retVal.upper_[i]= upper_[i];
            retVal.diag_[i] = diag_[i]+u[i]; 
        }

        return retVal;
    }

    Disposable<Array> TripleBandLinearOp::apply(const Array& r) const {
        const boost::shared_ptr<FdmLinearOpLayout> index = mesher_->layout();

        QL_REQUIRE(r.size() == index->size(), "inconsistent length of r");

        Array retVal(r.size());
        for (Size i=0; i < index->size(); ++i) {
            retVal[i] = r[i0_[i]]*lower_[i]+r[i]*diag_[i]+r[i2_[i]]*upper_[i];
        }

        return retVal;
    }

    Disposable<Array> 
    TripleBandLinearOp::solve_splitting(const Array& r, Real a, Real b) const {
        Array retVal(r.size());

        // start scrambling
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();

        std::vector<Size> newDim(layout->dim());
        std::iter_swap(newDim.begin(), newDim.begin()+direction_);
        std::vector<Size> newSpacing = FdmLinearOpLayout(newDim).spacing();
        std::iter_swap(newSpacing.begin(), newSpacing.begin()+direction_);

        Array diag(layout->size()), rhs(layout->size());
        Array lower(layout->size()-1, 0.0), upper(layout->size()-1, 0.0);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter!=endIter; ++iter) {
            const std::vector<Size>& coordinates = iter.coordinates();
            const Size index = iter.index();
            const Size newIndex 
                = std::inner_product(coordinates.begin(), coordinates.end(),
                                     newSpacing.begin(), Size(0)); 

            rhs[newIndex]   = r[index];
            diag[newIndex]  = a*diag_[index]+b;
            if (coordinates[direction_] != 0)
                lower[newIndex-1] = a*lower_[index];
            else 
                QL_REQUIRE(lower_[index] == 0, "removing non zero entry!");

            if (coordinates[direction_] != layout->dim()[direction_]-1)
                upper[newIndex] = a*upper_[index];
            else
                QL_REQUIRE(upper_[index] == 0, "removing non zero entry!");
        }

        Array s = TridiagonalOperator(lower, diag, upper).solveFor(rhs);

        for (FdmLinearOpIterator iter = layout->begin(); iter!=endIter; ++iter) {
            const std::vector<Size>& coordinates = iter.coordinates();
            const Size index = iter.index();
            const Size newIndex 
                = std::inner_product(coordinates.begin(), coordinates.end(),
                                     newSpacing.begin(), Size(0)); 

            retVal[index]   = s[newIndex];
        }

        return retVal;        
    }
}
