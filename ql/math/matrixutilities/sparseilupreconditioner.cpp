/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2009 Ralph Schreyer

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

#include <ql/math/matrixutilities/sparseilupreconditioner.hpp>
#include <ql/math/matrix.hpp>

#include <set>

namespace QuantLib {

    using namespace boost::numeric::ublas;

    SparseILUPreconditioner::SparseILUPreconditioner(const SparseMatrix& A,
                                                     Integer lfil)
    : L_(A.size1(),A.size2()),
      U_(A.size1(),A.size2()) {

        QL_REQUIRE(A.size1() == A.size2(),
                   "sparse ILU preconditioner works only with square matrices");

        for (SparseMatrix::size_type i=0; i < L_.size1(); ++i)
            L_(i,i) = 1.0;

        const Integer n = A.size1();
        std::set<Integer> lBandSet, uBandSet;

        compressed_matrix<Integer> levs(n,n);
        Integer lfilp = lfil + 1;

        for (Integer ii=0; ii<n; ++ii) {
            Array w(n);
            for(Integer k=0; k<n; ++k) {
                w[k] = A(ii,k);
            }

            std::vector<Integer> levii(n, 0);
            for (Integer i=0; i<n; ++i) {
                if(   w[i] > QL_EPSILON
                      || w[i] < -1.0*QL_EPSILON) levii[i] = 1;
            }
            Integer jj = -1;
            while (jj < ii) {
                for (Integer k=jj+1; k<n; ++k) {
                    if (levii[k] != 0) {
                        jj = k;
                        break;
                    }
                }
                if (jj >= ii) {
                    break;
                }
                Integer jlev = levii[jj];
                if (jlev <= lfilp) {
                    std::vector<Integer> nonZeros;
                    std::vector<Real> nonZeroEntries;
                    nonZeros.reserve(uBandSet.size()+1);
                    nonZeroEntries.reserve(uBandSet.size()+1);
                    const Real entry = U_(jj,jj);
                    if(entry > QL_EPSILON || entry < -1.0*QL_EPSILON) {
                        nonZeros.push_back(jj);
                        nonZeroEntries.push_back(entry);
                    }
                    auto iter = uBandSet.begin();
                    auto end = uBandSet.end();
                    for (; iter != end; ++iter) {
                        const Real entry = U_(jj,jj+*iter);
                        if(entry > QL_EPSILON || entry < -1.0*QL_EPSILON) {
                            nonZeros.push_back(jj+*iter);
                            nonZeroEntries.push_back(entry);
                        }
                    }
                    Real fact = w[jj];
                    if(!nonZeroEntries.empty()) {
                        fact /= nonZeroEntries[0];
                    }
                    for (Size k=0; k<nonZeros.size(); ++k) {
                        const Integer j = nonZeros[k] ;
                        const Integer temp = levs(jj,j) + jlev ;
                        if (levii[j] == 0) {
                            if (temp <= lfilp) {
                                w[j] =  - fact*nonZeroEntries[k];
                                levii[j] = temp;
                            }
                        }
                        else {
                            w[j] -= fact*nonZeroEntries[k];
                            levii[j] = std::min(levii[j],temp);
                        }
                    }
                    w[jj] = fact;
                }
            }
            std::vector<Integer> wNonZeros;
            std::vector<Real> wNonZeroEntries;
            wNonZeros.reserve(w.size());
            wNonZeroEntries.reserve(w.size());
            for (Size i=0; i<w.size(); ++i) {
                const Real entry = w[i];
                if(entry > QL_EPSILON || entry < -1.0*QL_EPSILON) {
                    wNonZeros.push_back(i);
                    wNonZeroEntries.push_back(entry);
                }
            }
            std::vector<Integer> leviiNonZeroEntries;
            leviiNonZeroEntries.reserve(levii.size());
            for (int entry : levii) {
                if (entry > QL_EPSILON || entry < -1.0 * QL_EPSILON) {
                    leviiNonZeroEntries.push_back(entry);
                }
            }
            for (Size k=0; k<wNonZeros.size(); ++k) {
                Integer j = wNonZeros[k];
                if (j < ii) {
                    L_(ii,j) = wNonZeroEntries[k];
                    lBandSet.insert(ii-j);
                }
                else {
                    U_(ii,j) = wNonZeroEntries[k];
                    levs(ii,j) = leviiNonZeroEntries[k];
                    if(j-ii > 0) {
                        uBandSet.insert(j-ii);
                    }
                }
            }
        }
        lBands_.resize(lBandSet.size());
        uBands_.resize(uBandSet.size());
        std::copy(lBandSet.begin(), lBandSet.end(), lBands_.begin());
        std::copy(uBandSet.begin(), uBandSet.end(), uBands_.begin());
    }

    const SparseMatrix& SparseILUPreconditioner::L() const {
        return L_;
    }

    const SparseMatrix& SparseILUPreconditioner::U() const {
        return U_;
    }

    Disposable<Array> SparseILUPreconditioner::apply(const Array& b) const {
        return backwardSolve(forwardSolve(b));
    }

    Disposable<Array> SparseILUPreconditioner::forwardSolve(
                                                       const Array& b) const {
        Integer n = b.size();
        Array y(n, 0.0);
        y[0]=b[0]/L_(0,0);
        for (Integer i=1; i<=n-1; ++i) {
            y[i] = b[i]/L_(i,i);
            for (Integer j=lBands_.size()-1;
                 j>=0 && i-Integer(lBands_[j]) <= i-1; --j) {
                const Integer k = i-Integer(lBands_[j]);
                if (k >= 0)
                    y[i]-=L_(i,k)*y[k]/L_(i,i);
            }
        }
        return y;
    }

    Disposable<Array> SparseILUPreconditioner::backwardSolve(
                                                       const Array& y) const {
        Size n = y.size();
        Array x(n, 0.0);
        x[n-1] = y[n-1]/U_(n-1,n-1);
        for (Integer i=n-2; i>=0; --i) {
            x[i] = y[i]/U_(i,i);
            for (Size j=0; j<uBands_.size() && i+uBands_[j] <= n-1; ++j) {
                x[i] -= U_(i,i+uBands_[j])*x[i+uBands_[j]]/U_(i,i);
            }
        }
        return x;
    }

}

