/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

#ifndef quantlib_alpha_form_concrete_hpp
#define quantlib_alpha_form_concrete_hpp

#include <ql/models/marketmodels/models/alphaform.hpp>
#include <vector>

namespace QuantLib {

    class AlphaFormInverseLinear : public AlphaForm {
      public:
        AlphaFormInverseLinear(std::vector<Time> times, Real alpha = 0.0);
        ~AlphaFormInverseLinear() override = default;
        Real operator()(Integer i) const override;
        void setAlpha(Real alpha_) override;

      private:
        std::vector<Time> times_;
        Real alpha_;
    };

    class AlphaFormLinearHyperbolic : public AlphaForm {
      public:
        AlphaFormLinearHyperbolic(std::vector<Time> times, Real alpha = 0.0);
        ~AlphaFormLinearHyperbolic() override = default;
        Real operator()(Integer i) const override;
        void setAlpha(Real alpha_) override;

      private:
        std::vector<Time> times_;
        Real alpha_;
    };

}

#endif


#ifndef id_9e77964cda5aeea6451ad4c979fa1604
#define id_9e77964cda5aeea6451ad4c979fa1604
inline bool test_9e77964cda5aeea6451ad4c979fa1604(const int* i) {
    return i != nullptr;
}
#endif
