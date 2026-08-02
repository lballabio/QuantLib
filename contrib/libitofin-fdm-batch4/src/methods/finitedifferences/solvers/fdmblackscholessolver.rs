//! Black-Scholes finite-difference solver over a prepared descriptor.
//!
//! Port of `ql/methods/finitedifferences/solvers/fdmblackscholessolver.{hpp,cpp}`.
//! Local-vol and quanto branches are deferred to #636 and omitted from the
//! constructor rather than accepted and ignored.

use std::cell::RefCell;

use crate::errors::QlResult;
use crate::methods::finitedifferences::operators::{FdmBlackScholesOp, FdmLinearOpComposite};
use crate::processes::GeneralizedBlackScholesProcess;
use crate::shared::{Shared, SharedMut, shared_mut};
use crate::types::Real;

use super::{Fdm1DimSolver, FdmSchemeDesc, FdmSolverDesc};

/// Prices a European option on a Black-Scholes process via finite differences.
///
/// Construction stores the process and descriptor (`fdmblackscholessolver.cpp:30-43`).
/// The first greek / value read builds an [`FdmBlackScholesOp`] and an
/// [`Fdm1DimSolver`] (`cpp:45-55`) and then reads off the log-spot cubic
/// (`cpp:57-75`).
pub struct FdmBlackScholesSolver {
    process: Shared<GeneralizedBlackScholesProcess>,
    strike: Real,
    solver_desc: FdmSolverDesc,
    scheme_desc: FdmSchemeDesc,
    solver: RefCell<Option<Fdm1DimSolver>>,
}

impl FdmBlackScholesSolver {
    /// Builds the solver for `process` at `strike` over `solver_desc`
    /// (`fdmblackscholessolver.cpp:30-43`).
    ///
    /// The default scheme is Douglas, matching C++.
    pub fn new(
        process: Shared<GeneralizedBlackScholesProcess>,
        strike: Real,
        solver_desc: FdmSolverDesc,
        scheme_desc: FdmSchemeDesc,
    ) -> Self {
        FdmBlackScholesSolver {
            process,
            strike,
            solver_desc,
            scheme_desc,
            solver: RefCell::new(None),
        }
    }

    /// Option value at spot `s` (`cpp:57-60`).
    pub fn value_at(&self, s: Real) -> QlResult<Real> {
        self.ensure_calculated()?.interpolate_at(s.ln())
    }

    /// Delta at spot `s` (`cpp:62-65`).
    pub fn delta_at(&self, s: Real) -> QlResult<Real> {
        Ok(self.ensure_calculated()?.derivative_x(s.ln())? / s)
    }

    /// Gamma at spot `s` (`cpp:67-71`).
    pub fn gamma_at(&self, s: Real) -> QlResult<Real> {
        let solver = self.ensure_calculated()?;
        let log_s = s.ln();
        Ok((solver.derivative_xx(log_s)? - solver.derivative_x(log_s)?) / (s * s))
    }

    /// Theta at spot `s` (`cpp:73-75`).
    pub fn theta_at(&self, s: Real) -> QlResult<Real> {
        self.ensure_calculated()?.theta_at(s.ln())
    }

    fn ensure_calculated(&self) -> QlResult<std::cell::Ref<'_, Fdm1DimSolver>> {
        if self.solver.borrow().is_none() {
            let op = shared_mut(FdmBlackScholesOp::new(
                Shared::clone(&self.solver_desc.mesher),
                &self.process,
                self.strike,
                0,
            )?);
            let solver = Fdm1DimSolver::new(
                self.solver_desc.clone(),
                self.scheme_desc,
                op as SharedMut<dyn FdmLinearOpComposite>,
            );
            *self.solver.borrow_mut() = Some(solver);
        }
        Ok(std::cell::Ref::map(self.solver.borrow(), |s| {
            s.as_ref().expect("just calculated")
        }))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::instrument::Instrument;
    use crate::instruments::PlainVanillaPayoff;
    use crate::methods::finitedifferences::meshers::{
        FdmMesher, FdmMesherComposite, fdm_black_scholes_mesher,
    };
    use crate::methods::finitedifferences::stepconditions::FdmStepConditionComposite;
    use crate::methods::finitedifferences::utilities::{
        FdmInnerValueCalculator, fdm_log_inner_value,
    };
    use crate::option::OptionType;
    use crate::payoff::Payoff;
    use crate::pricingengines::vanilla::test_market;
    use crate::shared::shared;

    fn fixture() -> (test_market::Market, FdmBlackScholesSolver, Real) {
        let market = test_market::market();
        market.set(100.0, 0.05, 0.05, 0.20);
        let maturity = 1.0;
        let strike = 100.0;
        let equity = fdm_black_scholes_mesher(
            100,
            &market.process,
            maturity,
            strike,
            None,
            None,
            0.0001,
            1.5,
            Some((strike, 0.1)),
            &[],
            0.0,
        )
        .unwrap();
        let mesher = shared(FdmMesherComposite::new(vec![equity]));
        let payoff: Shared<dyn Payoff> = shared(PlainVanillaPayoff::new(OptionType::Call, strike));
        let calculator: Shared<dyn FdmInnerValueCalculator> = shared(fdm_log_inner_value(
            payoff,
            Shared::clone(&mesher) as Shared<dyn FdmMesher>,
            0,
        ));
        let condition = shared(FdmStepConditionComposite::new(&[], Vec::new()));
        let desc = FdmSolverDesc::new(
            Shared::clone(&mesher) as _,
            Vec::new(),
            condition,
            calculator,
            maturity,
            50,
            0,
        );
        let solver = FdmBlackScholesSolver::new(
            Shared::clone(&market.process),
            strike,
            desc,
            FdmSchemeDesc::douglas(),
        );
        (market, solver, strike)
    }

    #[test]
    fn value_at_spot_matches_analytic_within_a_loose_band() {
        let (market, solver, strike) = fixture();
        let fd = solver.value_at(100.0).unwrap();

        let expiry = test_market::today() + test_market::time_to_days(1.0);
        let mut option = market.option(OptionType::Call, strike, expiry);
        let analytic = option.npv().unwrap();

        let rel = (fd - analytic).abs() / analytic;
        assert!(rel < 5.0e-3, "fd={fd} analytic={analytic} rel={rel}");
    }

    #[test]
    fn greeks_at_spot_are_finite() {
        let (_market, solver, _) = fixture();
        let delta = solver.delta_at(100.0).unwrap();
        let gamma = solver.gamma_at(100.0).unwrap();
        let theta = solver.theta_at(100.0).unwrap();
        assert!(delta.is_finite() && gamma.is_finite() && theta.is_finite());
        assert!(delta > 0.0 && delta < 1.0);
    }
}
