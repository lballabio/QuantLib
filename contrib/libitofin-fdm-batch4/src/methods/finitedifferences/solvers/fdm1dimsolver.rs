//! One-dimensional finite-difference solver with cubic read-off.
//!
//! Port of `ql/methods/finitedifferences/solvers/fdm1dimsolver.{hpp,cpp}`.

use std::cell::RefCell;

use crate::errors::QlResult;
use crate::math::array::Array;
use crate::math::interpolations::Interpolation;
use crate::math::interpolations::cubic::{CubicInterpolation, MonotonicCubicNaturalSpline};
use crate::methods::finitedifferences::operators::FdmLinearOpComposite;
use crate::methods::finitedifferences::stepconditions::{
    FdmSnapshotCondition, FdmStepConditionComposite,
};
use crate::shared::{Shared, SharedMut, shared};
use crate::types::Real;
use crate::utilities::null::Null;

use super::{FdmBackwardSolver, FdmSchemeDesc, FdmSolverDesc};

/// Rolls a one-dimensional grid back and reads value / greeks off a cubic.
///
/// Construction seeds the terminal payoff and joins a theta-snapshot condition
/// onto the caller's step conditions (`fdm1dimsolver.cpp:32-51`). The first
/// read-off rolls the grid back through [`FdmBackwardSolver`] and builds a
/// [`MonotonicCubicNaturalSpline`] through the result (`cpp:54-65`); later
/// reads reuse that spline until a new solver is built.
pub struct Fdm1DimSolver {
    solver_desc: FdmSolverDesc,
    scheme_desc: FdmSchemeDesc,
    op: SharedMut<dyn FdmLinearOpComposite>,
    theta_condition: Shared<FdmSnapshotCondition>,
    conditions: Shared<FdmStepConditionComposite>,
    x: Vec<Real>,
    initial_values: Vec<Real>,
    state: RefCell<Option<Solved>>,
}

struct Solved {
    #[allow(dead_code)] // retained to mirror C++'s `resultValues_`
    result_values: Array,
    interpolation: CubicInterpolation,
}

impl Fdm1DimSolver {
    /// Builds the solver over `op` for `solver_desc` / `scheme_desc`
    /// (`fdm1dimsolver.cpp:32-51`).
    pub fn new(
        solver_desc: FdmSolverDesc,
        scheme_desc: FdmSchemeDesc,
        op: SharedMut<dyn FdmLinearOpComposite>,
    ) -> Self {
        let first_stopping = solver_desc
            .condition
            .stopping_times()
            .first()
            .copied()
            .unwrap_or(solver_desc.maturity);
        let theta_time = 0.99 * Real::min(1.0 / 365.0, first_stopping);
        let theta_condition = shared(FdmSnapshotCondition::new(theta_time));
        let conditions =
            FdmStepConditionComposite::join_conditions(&theta_condition, &solver_desc.condition);

        let layout = solver_desc.mesher.layout();
        let size = layout.size();
        let mut x = vec![0.0; size];
        let mut initial_values = vec![0.0; size];
        for iter in layout.iter() {
            let index = iter.index();
            initial_values[index] = solver_desc
                .calculator
                .avg_inner_value(&iter, solver_desc.maturity);
            x[index] = solver_desc.mesher.location(&iter, 0);
        }

        Fdm1DimSolver {
            solver_desc,
            scheme_desc,
            op,
            theta_condition,
            conditions,
            x,
            initial_values,
            state: RefCell::new(None),
        }
    }

    /// Interpolated solution at grid coordinate `x` (`cpp:67-70`).
    pub fn interpolate_at(&self, x: Real) -> QlResult<Real> {
        let state = self.ensure_calculated()?;
        state.interpolation.value(x)
    }

    /// First derivative of the solution at `x` (`cpp:88-91`).
    pub fn derivative_x(&self, x: Real) -> QlResult<Real> {
        let state = self.ensure_calculated()?;
        state.interpolation.derivative(x)
    }

    /// Second derivative of the solution at `x` (`cpp:93-96`).
    pub fn derivative_xx(&self, x: Real) -> QlResult<Real> {
        let state = self.ensure_calculated()?;
        state.interpolation.second_derivative(x)
    }

    /// Finite-difference theta at `x` (`cpp:72-85`).
    ///
    /// Returns [`Real::null`] when the first stopping time is exactly zero, as
    /// C++ does with `Null<Real>()`.
    pub fn theta_at(&self, x: Real) -> QlResult<Real> {
        if self.conditions.stopping_times().first().copied() == Some(0.0) {
            return Ok(Real::null());
        }

        let state = self.ensure_calculated()?;
        let theta_values = self.theta_condition.values();
        let temp =
            MonotonicCubicNaturalSpline::new(self.x.clone(), theta_values.to_vec())?.value(x)?;
        let value = state.interpolation.value(x)?;
        Ok((temp - value) / self.theta_condition.time())
    }

    fn ensure_calculated(&self) -> QlResult<std::cell::Ref<'_, Solved>> {
        if self.state.borrow().is_none() {
            let mut rhs = Array::from(self.initial_values.clone());
            let mut solver = FdmBackwardSolver::new(
                SharedMut::clone(&self.op),
                self.solver_desc.bc_set.clone(),
                Some(Shared::clone(&self.conditions)),
                self.scheme_desc,
            );
            solver.rollback(
                &mut rhs,
                self.solver_desc.maturity,
                0.0,
                self.solver_desc.time_steps,
                self.solver_desc.damping_steps,
            )?;
            let interpolation = MonotonicCubicNaturalSpline::new(self.x.clone(), rhs.to_vec())?;
            *self.state.borrow_mut() = Some(Solved {
                result_values: rhs,
                interpolation,
            });
        }
        Ok(std::cell::Ref::map(self.state.borrow(), |s| {
            s.as_ref().expect("just calculated")
        }))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::instruments::PlainVanillaPayoff;
    use crate::methods::finitedifferences::meshers::{
        FdmMesher, FdmMesherComposite, fdm_black_scholes_mesher,
    };
    use crate::methods::finitedifferences::operators::FdmBlackScholesOp;
    use crate::methods::finitedifferences::utilities::{
        FdmInnerValueCalculator, fdm_log_inner_value,
    };
    use crate::option::OptionType;
    use crate::payoff::Payoff;
    use crate::pricingengines::vanilla::test_market;
    use crate::shared::shared_mut;
    use crate::types::Size;

    fn european_solver() -> Fdm1DimSolver {
        let market = test_market::market();
        market.set(100.0, 0.05, 0.05, 0.20);
        let maturity = 1.0;
        let strike = 100.0;
        let x_grid: Size = 100;
        let t_grid: Size = 50;

        let equity = fdm_black_scholes_mesher(
            x_grid,
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
            t_grid,
            0,
        );
        let op = shared_mut(
            FdmBlackScholesOp::new(Shared::clone(&mesher) as _, &market.process, strike, 0)
                .unwrap(),
        );
        Fdm1DimSolver::new(desc, FdmSchemeDesc::douglas(), op as _)
    }

    #[test]
    fn interpolate_at_the_spot_is_positive_for_an_atm_call() {
        let solver = european_solver();
        let value = solver.interpolate_at(100.0_f64.ln()).unwrap();
        assert!(value > 0.0, "atm call value was {value}");
        let delta = solver.derivative_x(100.0_f64.ln()).unwrap() / 100.0;
        assert!(delta > 0.0 && delta < 1.0, "atm call delta was {delta}");
    }

    #[test]
    fn theta_at_the_spot_is_finite() {
        let solver = european_solver();
        let theta = solver.theta_at(100.0_f64.ln()).unwrap();
        assert!(theta.is_finite());
        assert!(!theta.is_null());
    }
}
