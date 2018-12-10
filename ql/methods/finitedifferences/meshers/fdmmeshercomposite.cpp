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

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/assign/list_of.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

    namespace {
        typedef ext::shared_ptr<Fdm1dMesher> T;

        std::vector<T> build_vector(const T& m1) {
            return std::vector<T>(1, m1);
        }

        std::vector<T> build_vector(const T& m1, const T& m2) {
            const std::vector<ext::shared_ptr<Fdm1dMesher> > retVal
                = boost::assign::list_of(m1)(m2);
            return retVal;
        }

        std::vector<T> build_vector(const T& m1, const T& m2, const T& m3) {
            const std::vector<ext::shared_ptr<Fdm1dMesher> > retVal
                = boost::assign::list_of(m1)(m2)(m3);
            return retVal;
        }

        std::vector<T> build_vector(const T& m1, const T& m2,
                                    const T& m3, const T& m4) {
            const std::vector<ext::shared_ptr<Fdm1dMesher> > retVal
                = boost::assign::list_of(m1)(m2)(m3)(m4);
            return retVal;
        }

        ext::shared_ptr<FdmLinearOpLayout> getLayoutFromMeshers(
            const std::vector<ext::shared_ptr<Fdm1dMesher> > & meshers) {
                std::vector<Size> dim(meshers.size());
                for (Size i=0; i < dim.size(); ++i) {
                    dim[i] = meshers[i]->size();
                }
            return ext::make_shared<FdmLinearOpLayout>(
                dim);
        }
    }

    FdmMesherComposite::FdmMesherComposite(
        const ext::shared_ptr<Fdm1dMesher>& mesher)
    : FdmMesher(getLayoutFromMeshers(build_vector(mesher))),
      mesher_(build_vector(mesher)) {
    }


    FdmMesherComposite::FdmMesherComposite(
        const ext::shared_ptr<Fdm1dMesher>& m1,
        const ext::shared_ptr<Fdm1dMesher>& m2)
    : FdmMesher(getLayoutFromMeshers(build_vector(m1, m2))),
      mesher_(build_vector(m1, m2)) {
    }

    FdmMesherComposite::FdmMesherComposite(
        const ext::shared_ptr<Fdm1dMesher>& m1,
        const ext::shared_ptr<Fdm1dMesher>& m2,
        const ext::shared_ptr<Fdm1dMesher>& m3)
    : FdmMesher(getLayoutFromMeshers(build_vector(m1, m2, m3))),
      mesher_(build_vector(m1, m2, m3)) {
    }

    FdmMesherComposite::FdmMesherComposite(
        const ext::shared_ptr<Fdm1dMesher>& m1,
        const ext::shared_ptr<Fdm1dMesher>& m2,
        const ext::shared_ptr<Fdm1dMesher>& m3,
        const ext::shared_ptr<Fdm1dMesher>& m4)
    : FdmMesher(getLayoutFromMeshers(build_vector(m1, m2, m3, m4))),
      mesher_(build_vector(m1, m2, m3, m4)) {
    }
    FdmMesherComposite::FdmMesherComposite(
        const std::vector<ext::shared_ptr<Fdm1dMesher> > & mesher)
    : FdmMesher(getLayoutFromMeshers(mesher)), mesher_(mesher) {
    }

    FdmMesherComposite::FdmMesherComposite(
        const ext::shared_ptr<FdmLinearOpLayout>& layout,
        const std::vector<ext::shared_ptr<Fdm1dMesher> > & mesher)
    : FdmMesher(layout), mesher_(mesher) {
        for (Size i=0; i < mesher.size(); ++i) {
            QL_REQUIRE(mesher[i]->size() == layout->dim()[i],
                       "size of 1d mesher " << i << " does not fit to layout");
        }
    }

    Real FdmMesherComposite::dplus(const FdmLinearOpIterator& iter,
                                   Size direction) const {
        return mesher_[direction]->dplus(iter.coordinates()[direction]);
    }

    Real FdmMesherComposite::dminus(const FdmLinearOpIterator& iter,
                                    Size direction) const {
        return mesher_[direction]->dminus(iter.coordinates()[direction]);
    }

    Real FdmMesherComposite::location(const FdmLinearOpIterator& iter,
                                      Size direction) const {
        return mesher_[direction]->location(iter.coordinates()[direction]);
    }

    Disposable<Array> FdmMesherComposite::locations(Size direction) const {
        Array retVal(layout_->size());

        const FdmLinearOpIterator endIter = layout_->end();
        for (FdmLinearOpIterator iter = layout_->begin();
             iter != endIter; ++iter) {
            retVal[iter.index()] =
                mesher_[direction]->locations()[iter.coordinates()[direction]];
        }

        return retVal;
    }

    const std::vector<ext::shared_ptr<Fdm1dMesher> >&
        FdmMesherComposite::getFdm1dMeshers() const {
        return  mesher_;
    }
}
