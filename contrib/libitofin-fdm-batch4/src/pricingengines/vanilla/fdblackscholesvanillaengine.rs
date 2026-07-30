//! Finite-differences Black-Scholes vanilla option engine.
//!
//! Port of `ql/pricingengines/vanilla/fdblackscholesvanillaengine.{hpp,cpp}` for
//! the plain European / no-dividend path. Local-vol, quanto, cash-dividend and
//! American/Bermudan arms are deferred to #636 and omitted from the API rather
//! than accepted and ignored.

use crate::errors::QlResult;
use crate::exercise::ExerciseType;
use crate::fail;
use crate::instruments::{Greeks, OneAssetOptionEngine, OneAssetOptionResults, OptionArguments};
use crate::methods::finitedifferences::meshers::{
    FdmMesher, FdmMesherComposite, fdm_black_scholes_mesher,
};
use crate::methods::finitedifferences::solvers::{
    FdmBlackScholesSolver, FdmSchemeDesc, FdmSolverDesc,
};
use crate::methods::finitedifferences::stepconditions::FdmStepConditionComposite;
use crate::methods::finitedifferences::utilities::{FdmInnerValueCalculator, fdm_log_inner_value};
use crate::patterns::observable::AsObservable;
use crate::payoff::Payoff;
use crate::pricingengine::{Arguments, PricingEngine, Results};
use crate::processes::GeneralizedBlackScholesProcess;
use crate::shared::{Shared, shared};
use crate::stochasticprocess::StochasticProcess1D;
use crate::types::Size;
use crate::utilities::null::Null;

/// Finite-differences Black-Scholes engine for European vanilla options.
///
/// Defaults match C++ (`fdblackscholesvanillaengine.hpp:52-60`): `t_grid = 100`,
/// `x_grid = 100`, `damping_steps = 0`, Douglas scheme.
pub struct FdBlackScholesVanillaEngine {
    base: OneAssetOptionEngine,
    process: Shared<GeneralizedBlackScholesProcess>,
    t_grid: Size,
    x_grid: Size,
    damping_steps: Size,
    scheme_desc: FdmSchemeDesc,
}

impl FdBlackScholesVanillaEngine {
    /// Builds the engine on `process` with QuantLib's default grid and scheme.
    pub fn new(process: Shared<GeneralizedBlackScholesProcess>) -> Self {
        Self::with_grid(process, 100, 100, 0, FdmSchemeDesc::douglas())
    }

    /// Builds the engine with an explicit grid and scheme
    /// (`fdblackscholesvanillaengine.cpp:37-50`).
    pub fn with_grid(
        process: Shared<GeneralizedBlackScholesProcess>,
        t_grid: Size,
        x_grid: Size,
        damping_steps: Size,
        scheme_desc: FdmSchemeDesc,
    ) -> Self {
        let base =
            OneAssetOptionEngine::new(OptionArguments::default(), OneAssetOptionResults::default());
        base.register_with(process.observable());
        FdBlackScholesVanillaEngine {
            base,
            process,
            t_grid,
            x_grid,
            damping_steps,
            scheme_desc,
        }
    }
}

impl AsObservable for FdBlackScholesVanillaEngine {
    fn observable(&self) -> &crate::patterns::observable::Observable {
        self.base.observable()
    }
}

impl PricingEngine for FdBlackScholesVanillaEngine {
    fn arguments_mut(&mut self) -> &mut dyn Arguments {
        self.base.arguments_mut()
    }

    fn results(&self) -> &dyn Results {
        self.base.results()
    }

    fn reset(&mut self) {
        self.base.reset();
    }

    fn calculate(&mut self) -> QlResult<()> {
        let arguments = self.base.arguments();
        let Some(exercise) = &arguments.exercise else {
            fail!("no exercise given");
        };
        if exercise.exercise_type() != ExerciseType::European {
            fail!("not an European option");
        }
        let Some(payoff) = &arguments.payoff else {
            fail!("no payoff given");
        };
        let strike = payoff.strike();
        let maturity_date = exercise.last_date();
        let maturity = self.process.time(&maturity_date)?;

        // 1. Mesher (`fdblackscholesvanillaengine.cpp:154-167`). Quanto and
        // dividend-schedule arms are #636; European / no-dividend uses an empty
        // schedule and no spot adjustment.
        let equity_mesher = fdm_black_scholes_mesher(
            self.x_grid,
            &self.process,
            maturity,
            strike,
            None,
            None,
            0.0001,
            1.5,
            Some((strike, 0.1)),
            &[],
            0.0,
        )?;
        let mesher = shared(FdmMesherComposite::new(vec![equity_mesher]));

        // 2. Calculator (`cpp:169-171`).
        let payoff_as_payoff: Shared<dyn Payoff> = Shared::clone(payoff) as Shared<dyn Payoff>;
        let calculator: Shared<dyn FdmInnerValueCalculator> = shared(fdm_log_inner_value(
            payoff_as_payoff,
            Shared::clone(&mesher) as Shared<dyn FdmMesher>,
            0,
        ));

        // 3. Step conditions (`cpp:186-192`). Full `vanillaComposite` (American /
        // Bermudan / dividends) is #636; the European / no-dividend path is an
        // empty composite.
        let conditions = shared(FdmStepConditionComposite::new(&[], Vec::new()));

        // 4. Boundary conditions (`cpp:194-195`): empty.
        let boundaries = Vec::new();

        // 5. Solver (`cpp:197-214`).
        let solver_desc = FdmSolverDesc::new(
            Shared::clone(&mesher) as _,
            boundaries,
            conditions,
            calculator,
            maturity,
            self.t_grid,
            self.damping_steps,
        );
        let solver = FdmBlackScholesSolver::new(
            Shared::clone(&self.process),
            strike,
            solver_desc,
            self.scheme_desc,
        );

        let spot = self.process.x0()?;
        let value = solver.value_at(spot)?;
        let delta = solver.delta_at(spot)?;
        let gamma = solver.gamma_at(spot)?;
        let theta = solver.theta_at(spot)?;

        let results = self.base.results_mut();
        results.instrument.value = Some(value);
        results.greeks = Greeks {
            delta: Some(delta),
            gamma: Some(gamma),
            theta: if theta.is_null() { None } else { Some(theta) },
            vega: None,
            rho: None,
            dividend_rho: None,
        };
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    //! Oracles for `testFdEngines` and the ported arms of `testPDESchemes`
    //! from `test-suite/europeanoption.cpp`.

    use super::*;

    use crate::exercise::EuropeanExercise;
    use crate::instrument::Instrument;
    use crate::instruments::{EuropeanOption, PlainVanillaPayoff};
    use crate::methods::finitedifferences::solvers::FdmSchemeType;
    use crate::option::OptionType;
    use crate::pricingengines::vanilla::test_market::{self, Market};
    use crate::shared::{SharedMut, shared, shared_mut};
    use crate::types::Real;

    fn relative_error(x1: Real, x2: Real, reference: Real) -> Real {
        if reference != 0.0 {
            (x1 - x2).abs() / reference
        } else {
            (x1 - x2).abs()
        }
    }

    fn fd_option(
        market: &Market,
        option_type: OptionType,
        strike: Real,
        expiry: crate::time::date::Date,
        t_grid: Size,
        x_grid: Size,
        scheme: FdmSchemeDesc,
    ) -> EuropeanOption {
        let payoff = shared(PlainVanillaPayoff::new(option_type, strike));
        let exercise = shared(EuropeanExercise::new(expiry));
        let mut option = EuropeanOption::new(payoff, exercise, Shared::clone(&market.settings));
        let engine = shared_mut(FdBlackScholesVanillaEngine::with_grid(
            Shared::clone(&market.process),
            t_grid,
            x_grid,
            0,
            scheme,
        ));
        option
            .base_mut()
            .set_pricing_engine(engine as SharedMut<dyn PricingEngine>);
        option
    }

    /// `testFdEngines` (`europeanoption.cpp:1241-1254`): FD vs analytic over
    /// the consistency grid, tolerances value 1e-4 / delta-gamma 1e-6 /
    /// theta 1e-3. Grid is QuantLib's 500×500 (`cpp:1246-1247`).
    #[test]
    fn fd_engines_match_analytic_within_quantlib_tolerances() {
        let market = test_market::market();
        let types = [OptionType::Call, OptionType::Put];
        let strikes = [75.0, 100.0, 125.0];
        let underlyings = [100.0];
        let q_rates = [0.00, 0.05];
        let r_rates = [0.01, 0.05, 0.15];
        let vols = [0.11, 0.50, 1.20];
        let today = test_market::today();
        let expiry = today + 360;

        let tol_value = 1.0e-4;
        let tol_delta = 1.0e-6;
        let tol_gamma = 1.0e-6;
        let tol_theta = 1.0e-3;

        for option_type in types {
            for strike in strikes {
                let mut analytic = market.option(option_type, strike, expiry);
                let mut fd = fd_option(
                    &market,
                    option_type,
                    strike,
                    expiry,
                    500,
                    500,
                    FdmSchemeDesc::douglas(),
                );
                for u in underlyings {
                    for q in q_rates {
                        for r in r_rates {
                            for v in vols {
                                market.set(u, q, r, v);

                                let expected_value = analytic.npv().unwrap();
                                let calculated_value = fd.npv().unwrap();
                                let err = relative_error(expected_value, calculated_value, u);
                                assert!(
                                    err <= tol_value,
                                    "value type={option_type:?} K={strike} u={u} q={q} r={r} v={v}: \
                                     expected={expected_value} calculated={calculated_value} err={err}"
                                );

                                if calculated_value > u * 1.0e-5 {
                                    for (name, expected, calculated, tol) in [
                                        (
                                            "delta",
                                            analytic.delta().unwrap(),
                                            fd.delta().unwrap(),
                                            tol_delta,
                                        ),
                                        (
                                            "gamma",
                                            analytic.gamma().unwrap(),
                                            fd.gamma().unwrap(),
                                            tol_gamma,
                                        ),
                                        (
                                            "theta",
                                            analytic.theta().unwrap(),
                                            fd.theta().unwrap(),
                                            tol_theta,
                                        ),
                                    ] {
                                        let err = relative_error(expected, calculated, u);
                                        assert!(
                                            err <= tol,
                                            "{name} type={option_type:?} K={strike} u={u} q={q} r={r} v={v}: \
                                             expected={expected} calculated={calculated} err={err}"
                                        );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /// Ported arms of `testPDESchemes` (`europeanoption.cpp:1479+`): Douglas
    /// and ImplicitEuler only. The other scheme factories remain #636.
    #[test]
    fn ported_pde_schemes_match_analytic() {
        let market = test_market::market();
        market.set(100.0, 0.0, 0.05, 0.20);
        let today = test_market::today();
        let expiry = today + 365;
        let strike = 100.0;

        let mut analytic = market.option(OptionType::Call, strike, expiry);
        let expected = analytic.npv().unwrap();

        let schemes = [
            (FdmSchemeDesc::douglas(), 15, 100),
            (FdmSchemeDesc::implicit_euler(), 500, 100),
        ];

        for (scheme, t_grid, x_grid) in schemes {
            let mut option = fd_option(
                &market,
                OptionType::Call,
                strike,
                expiry,
                t_grid,
                x_grid,
                scheme,
            );
            let calculated = option.npv().unwrap();
            let err = relative_error(expected, calculated, 100.0);
            assert!(
                err <= 1.0e-3,
                "scheme={:?} expected={expected} calculated={calculated} err={err}",
                scheme.scheme_type
            );
            assert_ne!(scheme.scheme_type, FdmSchemeType::CrankNicolson);
        }
    }
}
