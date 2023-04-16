/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>

namespace QuantLib {
    FdmMesherIntegral::FdmMesherIntegral(
        const std::shared_ptr<FdmMesherComposite>& mesher,
        const std::function<Real(const Array&, const Array&)>& integrator1d)
    : meshers_(mesher->getFdm1dMeshers().begin(),
               mesher->getFdm1dMeshers().end()),
      integrator1d_(integrator1d) {
    }

    Real FdmMesherIntegral::integrate(const Array& f) const {
        const Array x(meshers_.back()->locations().begin(),
                      meshers_.back()->locations().end());

        if (meshers_.size() == 1) {
            return integrator1d_(x, f);
        }

        const std::shared_ptr<FdmMesherComposite> subMesher(
            new FdmMesherComposite(
                std::vector<std::shared_ptr<Fdm1dMesher> >(
                    meshers_.begin(), meshers_.end()-1)));

        FdmMesherIntegral subMesherIntegral(subMesher, integrator1d_);
        const Size subSize = subMesher->layout()->size();

        Array g(x.size()), fSub(subSize);

        for (Size i=0; i < x.size(); ++i) {
            std::copy(f.begin() + i    *subSize,
                      f.begin() + (i+1)*subSize, fSub.begin());

            g[i] = subMesherIntegral.integrate(fSub);
        }

        return integrator1d_(x, g);
    }
}
