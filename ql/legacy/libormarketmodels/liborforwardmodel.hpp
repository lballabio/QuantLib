/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file liborforwardmodel.hpp
    \brief libor forward model incl. exact cap pricing
           Rebonato formula to approximate swaption prices.
*/

#ifndef quantlib_libor_forward_model_hpp
#define quantlib_libor_forward_model_hpp

#include <ql/legacy/libormarketmodels/lfmprocess.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/optionlet/capletvariancecurve.hpp>
#include <ql/models/model.hpp>
#include <ql/legacy/libormarketmodels/lfmcovarproxy.hpp>

namespace QuantLib {

    //! %Libor forward model
    /*! References:

    Stefan Weber, 2005, Efficient Calibration for Libor Market Models,
    (<http://workshop.mathfinance.de/2005/papers/weber/slides.pdf>)

    Damiano Brigo, Fabio Mercurio, Massimo Morini, 2003,
    Different Covariance Parameterizations of Libor Market Model and Joint
    Caps/Swaptions Calibration,
    (<http://www.business.uts.edu.au/qfrc/conferences/qmf2001/Brigo_D.pdf>

    \test the correctness is tested using Monte-Carlo Simulation to
          reproduce swaption npvs, model calibration and exact cap pricing
    */

    class LiborForwardModel : public CalibratedModel, public AffineModel {
      public:
        LiborForwardModel(
            const ext::shared_ptr<LiborForwardModelProcess> & process,
            const ext::shared_ptr<LmVolatilityModel>  & volaModel,
            const ext::shared_ptr<LmCorrelationModel> & corrModel);

        Rate S_0(Size alpha, Size beta) const;
        // approx. swaption matrix using Rebonato's approx.
        // fix and floating leg have the same frequency
        virtual ext::shared_ptr<SwaptionVolatilityMatrix>
            getSwaptionVolatilityMatrix() const;

        DiscountFactor discount(Time t) const override;
        Real discountBond(Time now, Time maturity, Array factors) const override;
        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const override;

        void setParams(const Array& params) override;

      protected:
        Disposable<Array> w_0(Size alpha, Size beta) const;

        std::vector<Real> f_;
        std::vector<Time> accrualPeriod_;

        const ext::shared_ptr<LfmCovarianceProxy> covarProxy_;
        const ext::shared_ptr<LiborForwardModelProcess> process_;

        mutable ext::shared_ptr<SwaptionVolatilityMatrix> swaptionVola;
    };

}


#endif



#ifndef id_8577b592677a49e3359d0930ea771d7f
#define id_8577b592677a49e3359d0930ea771d7f
inline bool test_8577b592677a49e3359d0930ea771d7f(const int* i) {
    return i != nullptr;
}
#endif
