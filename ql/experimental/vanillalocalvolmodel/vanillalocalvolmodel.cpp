/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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


#include <sstream>

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/experimental/vanillalocalvolmodel/vanillalocalvolmodel.hpp>

namespace {

	// we need to calculate the cumulative normal distribution function (and derivative)
	// in various places
	QuantLib::CumulativeNormalDistribution Phi;

	// we need to convert numbers to strings for logging
	template <typename T>
	std::string to_string(T val) {
		std::stringstream stream;
		stream << val;
		return stream.str();
	}
}

namespace QuantLib {

	// we have two constructors and want to make sure the setup is consistent
	void VanillaLocalVolModel::initializeDeepInTheModelParameters() {
		straddleATM_ = sigmaATM_ * sqrt(T_) * M_1_SQRTPI * M_SQRT_2 * 2.0;
		if (useInitialMu_) mu_ = initialMu_;
		else mu_ = -(Mm_[0] + Mp_[0]) / 4.0 * T_; // this should be exact for shifted log-normal models
		alpha_ = 1.0;
		nu_ = 0.0;
		extrapolationStdevs_ = 10.0;
		sigma0Tol_ = 1.0e-12;
		S0Tol_ = 1.0e-12;
	}

    Real VanillaLocalVolModel::localVol(const bool isRightWing, const Size k, const Real S) {
		// this is an unsafe method specifying the vol function sigma(S) on the individual segments
		if (isRightWing) {
			QL_REQUIRE(k < sigmaP_.size(), "k < sigmaP_.size() required.");
			Real sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			Real S0 = (k > 0) ? Sp_[k - 1] : S0_;
			return sigma0 + Mp_[k] * (S - S0);
		} else {
			QL_REQUIRE(k < sigmaM_.size(), "k < sigmaM_.size() required.");
			Real sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			Real S0 = (k > 0) ? Sm_[k - 1] : S0_;
			return sigma0 + Mm_[k] * (S - S0);
		}
		return 0.0;  // this should never be reached
	}

	Real VanillaLocalVolModel::underlyingS(const bool isRightWing, const Size k, const Real x) {
		// this is an unsafe method specifying the underlying level S(x) on the individual segments
		if (isRightWing) {
			QL_REQUIRE(k < sigmaP_.size(), "k < sigmaP_.size() required.");
			Real sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			Real x0 = (k > 0) ? Xp_[k - 1] : 0.0;
			Real deltaS = (Mp_[k] == 0.0) ? (sigma0*(x - x0)) : (sigma0 / Mp_[k] * (exp(Mp_[k] * (x - x0)) - 1.0));
			Real S0 = (k > 0) ? Sp_[k - 1] : S0_;
			return S0 + deltaS;
		}
		else {
			QL_REQUIRE(k < sigmaM_.size(), "k < sigmaM_.size() required.");
			Real sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			Real x0 = (k > 0) ? Xm_[k - 1] : 0.0;
			Real deltaS = (Mm_[k] == 0.0) ? (sigma0*(x - x0)) : (sigma0 / Mm_[k] * (exp(Mm_[k] * (x - x0)) - 1.0));
			Real S0 = (k > 0) ? Sm_[k - 1] : S0_;
			return S0 + deltaS;
		}
		return 0.0;  // this should never be reached
	}

	Real VanillaLocalVolModel::underlyingX(const bool isRightWing, const Size k, const Real S) {
		// this is an unsafe method specifying the underlying level x(S) on the individual segments
		if (isRightWing) {
			QL_REQUIRE(k < sigmaP_.size(), "k < sigmaP_.size() required.");
			Real sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			QL_REQUIRE(sigma0 > 0.0, "sigma0 > 0.0 required");
			Real x0 = (k > 0) ? Xp_[k - 1] : 0.0;
			Real S0 = (k > 0) ? Sp_[k - 1] : S0_;
			Real deltaX = (Mp_[k] == 0.0) ? ((S - S0) / sigma0) : (log(1.0 + Mp_[k] / sigma0*(S - S0)) / Mp_[k]);
			return x0 + deltaX;
		}
		else {
			QL_REQUIRE(k < sigmaM_.size(), "k < sigmaM_.size() required.");
			Real sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			QL_REQUIRE(sigma0 > 0.0, "sigma0 > 0.0 required");
			Real x0 = (k > 0) ? Xm_[k - 1] : 0.0;
			Real S0 = (k > 0) ? Sm_[k - 1] : S0_;
			Real deltaX = (Mm_[k] == 0.0) ? ((S - S0) / sigma0) : (log(1.0 + Mm_[k] / sigma0*(S - S0)) / Mm_[k]);
			return x0 + deltaX;
		}
		return 0.0;  // this should never be reached
	}

	Real VanillaLocalVolModel::primitiveF(const bool isRightWing, const Size k, const Real x) {
		// this is an unsafe method specifying the primitive function F(x) = \int [alpha S(x) + nu] p(x) dx
		// on the individual segments
		Real sigma0, x0, S0, m0;
		if (isRightWing) {
			QL_REQUIRE(k < sigmaP_.size(), "k < sigmaP_.size() required.");
			sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			x0 = (k > 0) ? Xp_[k - 1] : 0.0;
			S0 = (k > 0) ? Sp_[k - 1] : S0_;
			m0 = Mp_[k];
		}
		else {
			QL_REQUIRE(k < sigmaM_.size(), "k < sigmaM_.size() required.");
			sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			x0 = (k > 0) ? Xm_[k - 1] : 0.0;
			S0 = (k > 0) ? Sm_[k - 1] : S0_;
			m0 = Mm_[k];
		}
		Real y0 = (x0 - mu_) / sqrt(T_);
		Real y1 = (x - mu_) / sqrt(T_);
		Real h = m0 * sqrt(T_);
		Real Ny = Phi(y1);
		Real term1, term2;
		if (m0 == 0.0) {
			term1 = (S0 + nu_ / alpha_ - sigma0 * sqrt(T_) * y0) * Ny;
			term2 = sigma0 * sqrt(T_) * Phi.derivative(y1);  // use dN/dx = dN/dy / sqrt(T)
		}
		else {				
			Real NyMinush = Phi(y1 - h);
			term1 = exp(h*h / 2.0 - h*y0)*sigma0 / m0 * NyMinush;
			term2 = (sigma0 / m0 - (S0 + nu_ / alpha_)) * Ny;
		}
		return alpha_ * (term1 - term2);
	}

	Real VanillaLocalVolModel::primitiveFSquare(const bool isRightWing, const Size k, const Real x) {
		// this is an unsafe method specifying the primitive function F(x) = \int [alpha S(x) + nu]^2 p(x) dx
		// on the individual segments
		Real sigma0, x0, S0, m0;
		if (isRightWing) {
			QL_REQUIRE(k < sigmaP_.size(), "k < sigmaP_.size() required.");
			sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			x0 = (k > 0) ? Xp_[k - 1] : 0.0;
			S0 = (k > 0) ? Sp_[k - 1] : S0_;
			m0 = Mp_[k];
		}
		else {
			QL_REQUIRE(k < sigmaM_.size(), "k < sigmaM_.size() required.");
			sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			x0 = (k > 0) ? Xm_[k - 1] : 0.0;
			S0 = (k > 0) ? Sm_[k - 1] : S0_;
			m0 = Mm_[k];
		}
		Real y0 = (x0 - mu_) / sqrt(T_);
		Real y1 = (x - mu_) / sqrt(T_);
		Real h = m0 * sqrt(T_);
		Real Ny = Phi(y1);
		Real sum = 0;
		if (m0 == 0.0) {
			Real K3 = S0 + nu_ / alpha_ - sigma0 * sqrt(T_) * y0;
			Real term1 = (K3 * K3 + sigma0 * sigma0 * T_) * Ny;
			Real term2 = 2.0 * sigma0 * sqrt(T_) * K3 + sigma0 * sigma0 * T_ * y1;
			term2 *= Phi.derivative(y1);  // use dN/dx = dN/dy / sqrt(T)
			sum = term1 - term2;
		}
		else {
			Real NyMinush  = Phi(y1 - h);
			Real NyMinus2h = Phi(y1 - 2.0*h);
			Real K1 = sigma0 / m0 * exp(h*(h - y0));
			Real K2 = S0 + nu_ / alpha_ - sigma0 / m0;
			Real term1 = K2 * K2 * Ny;
			Real term2 = 2.0 * K1 * K2 * exp(-h*h / 2.0) * NyMinush;
			Real term3 = K1 * K1 * NyMinus2h;
			sum = term1 + term2 + term3;
		}
		return alpha_ * alpha_ * sum;
	}

	void VanillaLocalVolModel::calculateSGrid() {
		// this is an unsafe method to calculate the S-grid for a given x-grid
        // it is intended as a preprocessing step in conjunction with smile interplation
		// validity of the model is ensured by proceeding it with updateLocalVol()
		for (Size k = 0; k < Xp_.size(); ++k) { // right wing calculations
			Sp_[k] = underlyingS(true, k, Xp_[k]);
			sigmaP_[k] = localVol(true, k, Sp_[k]);
		}
		for (Size k = 0; k < Sm_.size(); ++k) { // left wing calculations
			Sm_[k] = underlyingS(false, k, Xm_[k]);
			sigmaM_[k] = localVol(false, k, Sm_[k]);
		}
	}

	void VanillaLocalVolModel::updateLocalVol() {
		// use ODE solution to determine x-grid and sigma-grid taking into account constraints of
		// positive local volatility and local vol extrapolation
		for (Size k = 0; k < Sp_.size(); ++k) { // right wing calculations
			Real x0 = (k > 0) ? Xp_[k - 1] : 0.0;
			Real sigma0 = (k > 0) ? sigmaP_[k - 1] : sigma0_;
			QL_REQUIRE(sigma0 >= 0.0, "sigma0 >= 0.0 required.");
			if ((k == Sp_.size() - 1)||(localVol(true, k, Sp_[k])<=0.0)||(underlyingX(true, k, Sp_[k])>upperBoundX()))  { // right wing extrapolation, maybe better use some epsilon here
				Real XRight = upperBoundX();  // mu might not yet be calibrated
				QL_REQUIRE(XRight >= x0, "XRight >= x0 required.");
				Xp_[k] = XRight;
				Sp_[k] = underlyingS(true, k, XRight);
				sigmaP_[k] = localVol(true, k, Sp_[k]);
				if (k < Sp_.size() - 1) Mp_[k + 1] = Mp_[k];  // we need to make sure vol doesn't go up again
				continue;
			}
			sigmaP_[k] = localVol(true, k, Sp_[k]);
			QL_REQUIRE(sigmaP_[k] > 0.0, "sigmaP_[k] > 0.0 required.");
			Xp_[k] = underlyingX(true, k, Sp_[k]);
		}
		for (Size k = 0; k < Sm_.size(); ++k) { // left wing calculations
			Real x0 = (k > 0) ? Xm_[k - 1] : 0.0;
			Real sigma0 = (k > 0) ? sigmaM_[k - 1] : sigma0_;
			QL_REQUIRE(sigma0 >= 0.0, "sigma0 >= 0.0 required.");
			if ((k == Sm_.size() - 1)||(localVol(false, k, Sm_[k]) <= 0.0)||(underlyingX(false, k, Sm_[k])<lowerBoundX())) { // left wing extrapolation, maybe better use some epsilon here
				Real XLeft = lowerBoundX();  // mu might not yet be calibrated
				QL_REQUIRE(XLeft <= x0, "XLeft <= x0 required.");
				Xm_[k] = XLeft;
				Sm_[k] = underlyingS(false, k, XLeft);
				sigmaM_[k] = localVol(false, k, Sm_[k]);
				if (k < Sm_.size() - 1) Mm_[k + 1] = Mm_[k];  // we need to make sure vol doesn't go up again
				continue;
			}
			sigmaM_[k] = localVol(false, k, Sm_[k]);
			QL_REQUIRE(sigmaM_[k] > 0.0, "sigmaM_[k] > 0.0 required.");
			Xm_[k] = underlyingX(false, k, Sm_[k]);
		}
	}

	void VanillaLocalVolModel::calibrateATM() {
		Real straddleVega = straddleATM_ / sigmaATM_;
		Real forwardMinusStrike0, forwardMinusStrike1, straddleMinusATM0, straddleMinusATM1, dmu, dlogSigma0;
		Real	dfwd_dmu, dstr_dlogSigma0, logSigma0=log(sigma0_);
		for (Size k = 0; k < maxCalibrationIters_; ++k) {
			Real call = expectation(true, S0_);
			Real put = expectation(false, S0_);
			forwardMinusStrike1 = call - put;
			straddleMinusATM1 = call + put - straddleATM_;
			if (k > 0) {  // perform line search
				Real num = forwardMinusStrike0*(forwardMinusStrike1 - forwardMinusStrike0) +
					              straddleMinusATM0*(straddleMinusATM1 - straddleMinusATM0);
				Real den = (forwardMinusStrike1 - forwardMinusStrike0)*(forwardMinusStrike1 - forwardMinusStrike0) +
					              (straddleMinusATM1 - straddleMinusATM0)*(straddleMinusATM1 - straddleMinusATM0);
				Real lambda = -num / den;
				Real eps = 1.0e-6;  // see Griewank '86
				if (lambda < -0.5 - eps) lambda = -0.5;
				else if (lambda < -eps); // lambda = lambda;
				else if (lambda < 0.0) lambda = -eps;
				else if (lambda <= eps) lambda = eps;
				else if (lambda <= 0.5 + eps); // lambda = lambda;
				else lambda = 1.0;
				if (lambda < 1.0) { // reject the step and calculate a new try
					// x = x - dx + lambda dx = x + (lambda - 1.0) dx
					mu_ += (lambda - 1.0) * dmu;
					logSigma0 += (lambda - 1.0) * dlogSigma0;
					dmu *= lambda;
					dlogSigma0 *= lambda;
					sigma0_ = exp(logSigma0);
					updateLocalVol();
					if (enableLogging_) logging_.push_back("k: " + to_string(k) +
						"; C: " + to_string(call) +
						"; P: " + to_string(put) +
						"; S: " + to_string(straddleATM_) +
						"; lambda: " + to_string(lambda) +
						"; dmu: " + to_string(dmu) +
						"; dlogSigma0: " + to_string(dlogSigma0));
					continue;  // don't update derivatives and step direction for rejected steps
				}
			}
			if (k == 0) {
				dfwd_dmu = sigma0_;        // this is an estimate based on dS/dX at ATM
				dstr_dlogSigma0 = straddleVega * sigma0_;   // this is an estimate based on dsigmaATM / dsigma0 =~ 1
			}
			if (k>0) { // we use secant if available
				// only update derivative if we had a step, otherwise use from previous iteration
				// also avoid division by zero and zero derivative
				Real eps = 1.0e-12;  // we aim at beeing a bit more robust
				if ((fabs(forwardMinusStrike1 - forwardMinusStrike0) > eps) && (fabs(dmu) > eps)) {
					dfwd_dmu = (forwardMinusStrike1 - forwardMinusStrike0) / dmu;
				}
				if ((fabs(straddleMinusATM1 - straddleMinusATM0) > eps) && (fabs(dlogSigma0) > eps)) {
					dstr_dlogSigma0 = (straddleMinusATM1 - straddleMinusATM0) / dlogSigma0;
				}
			}
			dmu = -forwardMinusStrike1 / dfwd_dmu;
			if (k < onlyForwardCalibrationIters_) dlogSigma0 = 0.0;  // keep sigma0 fixed and only calibrate forward
			else dlogSigma0 = -straddleMinusATM1 / dstr_dlogSigma0;
			if (dmu <= -0.9*upperBoundX()) dmu = -0.5*upperBoundX();  // make sure 0 < eps < upperBoundX() in next update
			if (dmu >= -0.9*lowerBoundX()) dmu = -0.5*lowerBoundX();  // make sure 0 > eps > lowerBoundX() in next update
			// maybe some line search could improve convergence...
			mu_ += dmu;
			logSigma0 += dlogSigma0;
			sigma0_ = exp(logSigma0);  // ensure sigma0 > 0
			updateLocalVol();
			// prepare for next iteration
			forwardMinusStrike0 = forwardMinusStrike1;
			straddleMinusATM0 = straddleMinusATM1;
			if (enableLogging_) logging_.push_back("k: " + to_string(k) +
				"; C: " + to_string(call) +
				"; P: " + to_string(put) +
				"; S: " + to_string(straddleATM_) +
				"; dfwd_dmu: " + to_string(dfwd_dmu) +
				"; dstr_dlogSigma0: " + to_string(dstr_dlogSigma0) +
				"; dmu: " + to_string(dmu) +
				"; dlogSigma0: " + to_string(dlogSigma0));
			if ((fabs(forwardMinusStrike0) < S0Tol_) && (fabs(sigma0_*dlogSigma0) < sigma0Tol_)) break;
		}
	}

	void VanillaLocalVolModel::adjustATM() {
		// reset adjusters in case this method is invoked twice
		alpha_ = 1.0;
		nu_ = 0.0;
		Real call0 = expectation(true, S0_);
		Real put0 = expectation(false, S0_);
		nu_ = put0 - call0;
		if (enableLogging_) logging_.push_back("C0: " + to_string(call0) + "; P0: " + to_string(put0) + "; nu: " + to_string(nu_));
		Real call1 = expectation(true, S0_);
		Real put1 = expectation(false, S0_);
		alpha_ = straddleATM_ / (call1 + put1);
		nu_ = alpha_*nu_ + (1.0 - alpha_)*S0_;
		if (enableLogging_) logging_.push_back("C1: " + to_string(call1) + "; P1: " + to_string(put1) + "; alpha_: " + to_string(alpha_) + "; nu_: " + to_string(nu_));
	}

	// construct model based on S-grid
	VanillaLocalVolModel::VanillaLocalVolModel(
			const Time                T,
			const Real                S0,
			const Real                sigmaATM,
			const std::vector<Real>&  Sp,
			const std::vector<Real>&  Sm,
			const std::vector<Real>&  Mp,
			const std::vector<Real>&  Mm,
			// controls for calibration
			const Size                maxCalibrationIters,
			const Size                onlyForwardCalibrationIters,
			const bool                adjustATMFlag,
			const bool                enableLogging,
			const bool                useInitialMu,
			const Real                initialMu)
		: T_(T), S0_(S0), sigmaATM_(sigmaATM), Sp_(Sp), Sm_(Sm), Mp_(Mp), Mm_(Mm), sigma0_(sigmaATM),
		maxCalibrationIters_(maxCalibrationIters), onlyForwardCalibrationIters_(onlyForwardCalibrationIters),
		adjustATM_(adjustATMFlag), useInitialMu_(useInitialMu), initialMu_(initialMu), enableLogging_(enableLogging) {
		// some basic sanity checks come here to avoid the need for taking care of it later on
		QL_REQUIRE(T_ > 0, "T_ > 0 required.");
		QL_REQUIRE(sigmaATM_ > 0, "sigmaATM_ > 0 required.");
		QL_REQUIRE(Sp_.size() > 0, "Sp_.size() > 0 required.");
		QL_REQUIRE(Sm_.size() > 0, "Sm_.size() > 0 required.");
		QL_REQUIRE(Mp_.size() == Sp_.size(), "Mp_.size() == Sp_.size() required.");
		QL_REQUIRE(Mm_.size() == Sm_.size(), "Mm_.size() == Sm_.size() required.");
		// check for monotonicity
		QL_REQUIRE(Sp_[0] > S0_, "Sp_[0] > S0_ required.");
		for (Size k=1; k<Sp_.size(); ++k) QL_REQUIRE(Sp_[k] > Sp_[k-1], "Sp_[k] > Sp_[k-1] required.");
		QL_REQUIRE(Sm_[0] < S0_, "Sm_[0] < S0_ required.");
		for (Size k = 1; k<Sm_.size(); ++k) QL_REQUIRE(Sm_[k] < Sm_[k-1], "Sm_[k] < Sm_[k-1] required.");
		// now it makes sense to allocate memory
		sigmaP_.resize(Sp_.size());
		sigmaM_.resize(Sm_.size());
		Xp_.resize(Sp_.size());
		Xm_.resize(Sm_.size());
		// initialize deep-in-the-model parameters
		initializeDeepInTheModelParameters();
		// now we may calculate local volatility
		updateLocalVol();
		calibrateATM();
		if (adjustATM_) adjustATM();
	}

	// construct model based on x-grid
	VanillaLocalVolModel::VanillaLocalVolModel(
			const Time                T,
			const Real                S0,
			const Real                sigmaATM,
			const Real                sigma0,
			const std::vector<Real>&  Xp,
			const std::vector<Real>&  Xm,
			const std::vector<Real>&  Mp,
			const std::vector<Real>&  Mm,
			// controls for calibration
			const Size                maxCalibrationIters,
			const Size                onlyForwardCalibrationIters,
			const bool                adjustATMFlag,
			const bool                enableLogging,
			const bool                useInitialMu,
			const Real                initialMu)
			: T_(T), S0_(S0), sigmaATM_(sigmaATM), Mp_(Mp), Mm_(Mm), sigma0_(sigma0), Xp_(Xp), Xm_(Xm),
			maxCalibrationIters_(maxCalibrationIters), onlyForwardCalibrationIters_(onlyForwardCalibrationIters),
			adjustATM_(adjustATMFlag), useInitialMu_(useInitialMu), initialMu_(initialMu), enableLogging_(enableLogging) {
			// some basic sanity checks come here to avoid the need for taking care of it later on
			QL_REQUIRE(T_ > 0, "T_ > 0 required.");
			QL_REQUIRE(sigmaATM_ > 0, "sigmaATM_ > 0 required.");
			QL_REQUIRE(sigma0_ > 0, "sigma0_ > 0 required.");
			QL_REQUIRE(Xp_.size() > 0, "Xp_.size() > 0 required.");
			QL_REQUIRE(Xm_.size() > 0, "Xm_.size() > 0 required.");
			QL_REQUIRE(Mp_.size() == Xp_.size(), "Mp_.size() == Xp_.size() required.");
			QL_REQUIRE(Mm_.size() == Xm_.size(), "Mm_.size() == Xm_.size() required.");
			// check for monotonicity
			QL_REQUIRE(Xp_[0] > 0.0, "Xp_[0] > 0.0 required.");
			for (Size k = 1; k<Xp_.size(); ++k) QL_REQUIRE(Xp_[k] > Xp_[k - 1], "Xp_[k] > Xp_[k-1] required.");
			QL_REQUIRE(Xm_[0] < 0.0, "Xm_[0] < 0.0 required.");
			for (Size k = 1; k<Xm_.size(); ++k) QL_REQUIRE(Xm_[k] < Xm_[k - 1], "Xm_[k] < Xm_[k-1] required.");
			// now it makes sense to allocate memory
			sigmaP_.resize(Xp_.size());
			sigmaM_.resize(Xm_.size());
			Sp_.resize(Xp_.size());
			Sm_.resize(Xm_.size());
			// initialize deep-in-the-model parameters
			initializeDeepInTheModelParameters();
			// now we may calculate local volatility
			calculateSGrid();  // we need this preprocessing step since we only input x instead of S
			updateLocalVol();
			calibrateATM();
			if (adjustATM_) adjustATM();
		}

	// attributes in more convenient single-vector format

	const std::vector<Real> VanillaLocalVolModel::underlyingX() {
		std::vector<Real> X(Xm_.size() + Xp_.size() + 1);
		for (Size k = 0; k < Xm_.size(); ++k) X[k] = Xm_[Xm_.size() - k - 1];
		X[Xm_.size()] = 0.0;
		for (Size k = 0; k < Xp_.size(); ++k) X[Xm_.size() + 1 + k] = Xp_[k];
		return X;
	}

	const std::vector<Real> VanillaLocalVolModel::underlyingS() {
		std::vector<Real> S(Sm_.size() + Sp_.size() + 1);
		for (Size k = 0; k < Sm_.size(); ++k) S[k] = Sm_[Sm_.size() - k - 1];
		S[Sm_.size()] = S0_;
		for (Size k = 0; k < Sp_.size(); ++k) S[Sm_.size() + 1 + k] = Sp_[k];
		return S;
	}

	const std::vector<Real> VanillaLocalVolModel::localVol() {
		std::vector<Real> sigma(sigmaM_.size() + sigmaP_.size() + 1);
		for (Size k = 0; k < sigmaM_.size(); ++k) sigma[k] = sigmaM_[sigmaM_.size() - k - 1];
		sigma[sigmaM_.size()] = sigma0_;
		for (Size k = 0; k < sigmaP_.size(); ++k) sigma[sigmaM_.size() + 1 + k] = sigmaP_[k];
		return sigma;
	}

	const std::vector<Real> VanillaLocalVolModel::localVolSlope() {
		std::vector<Real> m(Mm_.size() + Mp_.size() + 1);
		for (Size k = 0; k < Mm_.size(); ++k) m[k] = Mm_[Mm_.size() - k - 1];
		m[Mm_.size()] = 0.0;  // undefined
		for (Size k = 0; k < Mp_.size(); ++k) m[Mm_.size() + 1 + k] = Mp_[k];
		return m;
	}

	// model function evaluations

	const Real VanillaLocalVolModel::localVol(Real S) {
		bool isRightWing = (S >= S0_) ? true : false;
		Size idx = 0;
		if (isRightWing) while ((idx < Sp_.size() - 1) && (Sp_[idx] < S)) ++idx;
		else             while ((idx < Sm_.size() - 1) && (Sm_[idx] > S)) ++idx;
		return localVol(isRightWing, idx, S);
	}

	const Real VanillaLocalVolModel::underlyingS(Real x) {
		bool isRightWing = (x >= 0.0) ? true : false;
		Size idx = 0;
		if (isRightWing) while ((idx < Xp_.size() - 1) && (Xp_[idx] < x)) ++idx;
		else             while ((idx < Xm_.size() - 1) && (Xm_[idx] > x)) ++idx;
		return underlyingS(isRightWing, idx, x);
	}

	// calculating expectations - that is the actual purpose of that model

	const Real VanillaLocalVolModel::expectation(bool isRightWing, Real strike) {
		// calculate the forward price of an OTM option
		Size idx = 0;
		if (isRightWing) {
			QL_REQUIRE(strike >= S0_, "strike >= S0_ required");
			while ((idx < Sp_.size()) && (Sp_[idx] <= strike)) ++idx;  // make sure strike < Sp_[idx]
			if (idx == Sp_.size()) return 0.0;  // we are beyond exrapolation
			Real strikeX = underlyingX(isRightWing, idx, strike);
			Real x0 = (idx > 0) ? Xp_[idx - 1] : 0.0;
			QL_REQUIRE((x0 <= strikeX) && (strikeX <= Xp_[idx]), "(x0 <= strikeX) && (strikeX <= Xp_[idx]) required");
			Real intS = 0.0;
			for (Size k = idx; k < Sp_.size(); ++k) {
				Real xStart = (k == idx) ? strikeX : Xp_[k - 1];
				intS += (primitiveF(isRightWing, k, Xp_[k]) - primitiveF(isRightWing, k, xStart));
			}
			// we need to adjust for the strike integral
			Real xEnd = Xp_.back();
			Real intK = Phi((xEnd - mu_) / sqrt(T_)) - Phi((strikeX - mu_) / sqrt(T_));
			return intS - strike * intK;
		}
		else {
			QL_REQUIRE(strike <= S0_, "strike <= S0_ required");
			while ((idx < Sm_.size()) && (Sm_[idx] >= strike)) ++idx;  // make sure Sm_[idx] < strke
			if (idx == Sm_.size()) return 0.0;  // we are beyond exrapolation
			Real strikeX = underlyingX(isRightWing, idx, strike);
			Real x0 = (idx > 0) ? Xm_[idx - 1] : 0.0;
			QL_REQUIRE((x0 >= strikeX) && (strikeX >= Xm_[idx]), "(x0 >= strikeX) && (strikeX >= Xm_[idx]) required");
			Real intS = 0.0;
			for (Size k = idx; k < Sm_.size(); ++k) {
				Real xStart = (k == idx) ? strikeX : Xm_[k - 1];
				intS += (primitiveF(isRightWing, k, Xm_[k]) - primitiveF(isRightWing, k, xStart));
			}
			// we need to adjust for the strike integral
			Real xEnd = Xm_.back();
			Real intK = Phi((xEnd - mu_) / sqrt(T_)) - Phi((strikeX - mu_) / sqrt(T_));
			return intS - strike * intK;
		}
	}

	const Real VanillaLocalVolModel::variance(bool isRightWing, Real strike) {
		// calculate the forward price of an OTM power option with payoff 1_{S>K}(S-K)^2
		Size idx = 0;
		if (isRightWing) {
			QL_REQUIRE(strike >= S0_, "strike >= S0_ required");
			while ((idx < Sp_.size()) && (Sp_[idx] <= strike)) ++idx;  // make sure strike < Sp_[idx]
			if (idx == Sp_.size()) return 0.0;  // we are beyond exrapolation
			Real strikeX = underlyingX(isRightWing, idx, strike);
			Real x0 = (idx > 0) ? Xp_[idx - 1] : 0.0;
			QL_REQUIRE((x0 <= strikeX) && (strikeX <= Xp_[idx]), "(x0 <= strikeX) && (strikeX <= Xp_[idx]) required");
			Real intS=0.0, intS2 = 0.0;
			for (Size k = idx; k < Sp_.size(); ++k) {
				Real xStart = (k == idx) ? strikeX : Xp_[k - 1];
				intS  += (primitiveF(isRightWing, k, Xp_[k]) - primitiveF(isRightWing, k, xStart));
				intS2 += (primitiveFSquare(isRightWing, k, Xp_[k]) - primitiveFSquare(isRightWing, k, xStart));
			}
			// we need to adjust for the Vanilla and strike integral
			Real xEnd = Xp_.back();
			Real intK = Phi((xEnd - mu_) / sqrt(T_)) - Phi((strikeX - mu_) / sqrt(T_));
			return intS2 - 2.0 * strike * intS + strike * strike * intK;
		}
		else {
			QL_REQUIRE(strike <= S0_, "strike <= S0_ required");
			while ((idx < Sm_.size()) && (Sm_[idx] >= strike)) ++idx;  // make sure Sm_[idx] < strke
			if (idx == Sm_.size()) return 0.0;  // we are beyond exrapolation
			Real strikeX = underlyingX(isRightWing, idx, strike);
			Real x0 = (idx > 0) ? Xm_[idx - 1] : 0.0;
			QL_REQUIRE((x0 >= strikeX) && (strikeX >= Xm_[idx]), "(x0 >= strikeX) && (strikeX >= Xm_[idx]) required");
			Real intS = 0.0, intS2 = 0.0;
			for (Size k = idx; k < Sm_.size(); ++k) {
				Real xStart = (k == idx) ? strikeX : Xm_[k - 1];
				intS  += (primitiveF(isRightWing, k, Xm_[k]) - primitiveF(isRightWing, k, xStart));
				intS2 += (primitiveFSquare(isRightWing, k, Xm_[k]) - primitiveFSquare(isRightWing, k, xStart));
			}
			// we need to adjust for the strike integral
			Real xEnd = Xm_.back();
			Real intK = Phi((xEnd - mu_) / sqrt(T_)) - Phi((strikeX - mu_) / sqrt(T_));
			return -(intS2 - 2.0 * strike * intS + strike * strike * intK);
		}
	}

}  // namespace QauntLib

