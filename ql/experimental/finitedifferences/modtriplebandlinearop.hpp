/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Klaus Spanderen

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

/*! \file modtriplebandlinearop.hpp
    \brief modifiable triple band linear operator
*/

#ifndef quantlib_mod_triple_band_linear_op_hpp
#define quantlib_mod_triple_band_linear_op_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>

namespace QuantLib {

    class ModTripleBandLinearOp : public TripleBandLinearOp {
      public:
        ModTripleBandLinearOp(Size direction,
                              const ext::shared_ptr<FdmMesher>& mesher)
        : TripleBandLinearOp(direction, mesher) { }

        explicit ModTripleBandLinearOp(const TripleBandLinearOp& m)
        : TripleBandLinearOp(m) { }

        Real lower(Size i) const { return lower_[i]; }
        Real& lower(Size i) { return lower_[i]; }
        Real diag(Size i) const { return diag_[i]; }
        Real& diag(Size i) { return diag_[i]; }
        Real upper(Size i) const { return upper_[i]; }
        Real& upper(Size i) { return upper_[i]; }
    };
}

#endif
