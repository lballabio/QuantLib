/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file smilesectionutils.hpp
    \brief Additional utilities for smile sections
*/

#ifndef quantlib_smile_section_utils_hpp
#define quantlib_smile_section_utils_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/utilities/disposable.hpp>
#include <vector>

namespace QuantLib {

    //! smile-section utilities
    class SmileSectionUtils {
      public:
        const std::pair<Real,Real>
        arbitragefreeRegion(const SmileSection&,
                            const std::vector<Real>& moneynessGrid =
                                                   std::vector<Real>()) const;
        const std::pair<Size,Size>
        arbitragefreeIndices(const SmileSection&,
                             const std::vector<Real>& moneynessGrid =
                                                   std::vector<Real>()) const;
        const std::vector<Real>&
        makeMoneynessGrid(const SmileSection&, 
                          const std::vector<Real>& moneynessGrid) const;
        
        const std::vector<Real>&
        makeStrikeGrid(const SmileSection&,
                       const std::vector<Real>& moneynessGrid) const;
      private:
        bool af(Size i0, Size i, Size i1) const;
        mutable std::vector<Real> m_, k_, c_;
    };

}

#endif
