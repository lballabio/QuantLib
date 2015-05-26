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

/*! \file localvolendcalculator.hpp
    \brief local volatility risk neutral terminal density calculation
*/

#ifndef quantlib_local_vol_rnd_calculator_hpp
#define quantlib_local_vol_rnd_calculator_hpp

#include <ql/math/matrix.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/experimental/finitedifferences/riskneutraldensitycalculator.hpp>

namespace QuantLib {
	class Quote;
	class TimeGrid;
	class YieldTermStructure;
	class LocalVolTermStructure;

	class LocalVolRNDCalculator : public RiskNeutralDensityCalculator,
								  public LazyObject {
	  public:
		LocalVolRNDCalculator(
			const boost::shared_ptr<Quote>& spot,
			const boost::shared_ptr<YieldTermStructure>& rTS,
			const boost::shared_ptr<YieldTermStructure>& qTS,
			const boost::shared_ptr<LocalVolTermStructure>& localVol,
			Size xGrid = 101, Size tGrid = 51, Real eps = 1e-10);

		LocalVolRNDCalculator(
			const boost::shared_ptr<Quote>& spot,
			const boost::shared_ptr<YieldTermStructure>& rTS,
			const boost::shared_ptr<YieldTermStructure>& qTS,
			const boost::shared_ptr<LocalVolTermStructure>& localVol,
			const boost::shared_ptr<TimeGrid>& timeGrid,
			Size xGrid = 101, Real eps = 1e-10);

		Real pdf(Real x, Time t) const;
		Real cdf(Real x, Time t) const;
		Real invcdf(Real p, Time t) const;

		Disposable<Array> x(Time t) const; // ln(s) grid
		boost::shared_ptr<TimeGrid> timeGrid() const;

	  protected:
		void performCalculations() const;

		static Disposable<Array> rescalePDF(const Array& x, const Array& p);

	  private:
		const Size xGrid_, tGrid_;
		const Real eps_;
		const boost::shared_ptr<Quote> spot_;
		const boost::shared_ptr<LocalVolTermStructure> localVol_;
		const boost::shared_ptr<YieldTermStructure> rTS_;
		const boost::shared_ptr<YieldTermStructure> qTS_;
		const boost::shared_ptr<TimeGrid> timeGrid_;
		const boost::shared_ptr<Matrix> x_, vols_;
	};
}

#endif
