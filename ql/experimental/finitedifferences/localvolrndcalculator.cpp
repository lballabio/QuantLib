/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file localvolendcalculator.cpp
    \brief local volatility risk neutral terminal density calculation
*/

#include <ql/quote.hpp>
#include <ql/timegrid.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/experimental/finitedifferences/localvolrndcalculator.hpp>

namespace QuantLib {
	LocalVolRNDCalculator::LocalVolRNDCalculator(
		const boost::shared_ptr<Quote>& spot,
		const boost::shared_ptr<YieldTermStructure>& rTS,
		const boost::shared_ptr<YieldTermStructure>& qTS,
		const boost::shared_ptr<LocalVolTermStructure>& localVol,
		Size xGrid, Size tGrid, Real eps)
	: xGrid_(xGrid),
	  tGrid_(tGrid),
	  eps_     (eps),
	  spot_	   (spot),
	  localVol_(localVol),
	  rTS_     (rTS),
	  qTS_     (qTS),
	  timeGrid_(new TimeGrid(localVol->maxTime(), tGrid)),
	  x_       (new Matrix(tGrid, xGrid)),
	  vols_    (new Matrix(tGrid, xGrid)) {
		registerWith(spot_);
		registerWith(rTS_);
		registerWith(qTS_);
		registerWith(localVol_);
	}

	LocalVolRNDCalculator::LocalVolRNDCalculator(
		const boost::shared_ptr<Quote>& spot,
		const boost::shared_ptr<YieldTermStructure>& rTS,
		const boost::shared_ptr<YieldTermStructure>& qTS,
		const boost::shared_ptr<LocalVolTermStructure>& localVol,
		const boost::shared_ptr<TimeGrid>& timeGrid,
		Size xGrid, Real eps)
	: xGrid_(xGrid),
	  tGrid_(timeGrid->size()-1),
	  eps_     (eps),
	  spot_    (spot),
	  localVol_(localVol),
	  rTS_     (rTS),
	  qTS_     (qTS),
	  timeGrid_(timeGrid),
	  x_       (new Matrix(tGrid_, xGrid_)),
	  vols_    (new Matrix(tGrid_, xGrid_)) {
		registerWith(spot_);
		registerWith(rTS_);
		registerWith(qTS_);
		registerWith(localVol_);
	}

	Real LocalVolRNDCalculator::pdf(Real x, Time t) const {
		calculate();
		return 1.0;
	}

	Real LocalVolRNDCalculator::cdf(Real x, Time t) const {
		calculate();
		return 1.0;
	}

	Real LocalVolRNDCalculator::invcdf(Real p, Time t) const {
		calculate();
		return 1.0;
	}

	Disposable<Array> LocalVolRNDCalculator::x(Time t) const {
        calculate();

		const Size idx = timeGrid_->index(t);
		QL_REQUIRE(idx <= x_->rows(), "inconsistent time " << t << " given");

		if (idx > 0) {
			Array retVal(x_->row_begin(idx-1), x_->row_end(idx-1));
			return retVal;
		}
		else {
			Array retVal(x_->columns(), std::log(spot_->value()));
			return retVal;
		}
	}

	boost::shared_ptr<TimeGrid> LocalVolRNDCalculator::timeGrid() const {
		return timeGrid_;
	}

	void LocalVolRNDCalculator::performCalculations() const {
		const Time sT = timeGrid_->at(1);
		const Time ep = std::min(sT, 1.0/365);

		const Volatility vol = localVol_->localVol(0.5*ep, spot_->value());

		const Volatility stdDev = vol * std::sqrt(sT);
		const Real xm = - 0.5 * stdDev * stdDev +
			std::log(spot_->value() * qTS_->discount(sT)/rTS_->discount(sT));

		const Real normInvEps = InverseCumulativeNormal()(1 - eps_);
		const Real sLowerBound = xm - normInvEps * stdDev;
		const Real sUpperBound = xm + normInvEps * stdDev;

		const boost::shared_ptr<Fdm1dMesher> mesher(
			new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid_,
				std::make_pair(xm, 0.1), true));

	    Array p(mesher->size());
	    GaussianDistribution gaussianPDF(xm, vol * std::sqrt(ep));
	    for (Size idx=0; idx < p.size(); ++idx) {
	        const Real x = mesher->location(idx);
	        p[idx] = gaussianPDF(x);
	    }


	}
}

