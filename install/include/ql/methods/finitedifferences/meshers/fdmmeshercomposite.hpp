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

/*! \file fdmmeshercomposite.hpp
    \brief FdmMesher which is a composite of Fdm1dMesher
*/

#ifndef quantlib_fdm_mesher_composite_hpp
#define quantlib_fdm_mesher_composite_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>

namespace QuantLib {

    class FdmMesherComposite : public FdmMesher {
      public:
        FdmMesherComposite(
            const ext::shared_ptr<FdmLinearOpLayout>& layout,
            const std::vector<ext::shared_ptr<Fdm1dMesher> > & mesher);

        // convenient constructors
        explicit FdmMesherComposite(
            const std::vector<ext::shared_ptr<Fdm1dMesher> > & mesher);
        explicit FdmMesherComposite(
            const ext::shared_ptr<Fdm1dMesher>& mesher);
        FdmMesherComposite(const ext::shared_ptr<Fdm1dMesher>& m1,
                           const ext::shared_ptr<Fdm1dMesher>& m2);
        FdmMesherComposite(const ext::shared_ptr<Fdm1dMesher>& m1,
                           const ext::shared_ptr<Fdm1dMesher>& m2,
                           const ext::shared_ptr<Fdm1dMesher>& m3);
        FdmMesherComposite(const ext::shared_ptr<Fdm1dMesher>& m1,
                           const ext::shared_ptr<Fdm1dMesher>& m2,
                           const ext::shared_ptr<Fdm1dMesher>& m3,
                           const ext::shared_ptr<Fdm1dMesher>& m4);


        Real dplus(const FdmLinearOpIterator& iter, Size direction) const override;
        Real dminus(const FdmLinearOpIterator& iter, Size direction) const override;
        Real location(const FdmLinearOpIterator& iter, Size direction) const override;
        Array locations(Size direction) const override;

        const std::vector<ext::shared_ptr<Fdm1dMesher> >&
            getFdm1dMeshers() const;

      private:
        const std::vector<ext::shared_ptr<Fdm1dMesher> > mesher_;
    };
}

#endif
