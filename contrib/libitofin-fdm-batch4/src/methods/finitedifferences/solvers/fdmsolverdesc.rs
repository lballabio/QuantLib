//! The inputs a finite-difference solver needs to roll a grid back.
//!
//! Port of `ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp:35-43`.

use crate::methods::finitedifferences::meshers::FdmMesher;
use crate::methods::finitedifferences::stepconditions::FdmStepConditionComposite;
use crate::methods::finitedifferences::utilities::{
    FdmBoundaryConditionSet, FdmInnerValueCalculator,
};
use crate::shared::Shared;
use crate::types::{Size, Time};

/// Everything a one-dimensional finite-difference solver needs to price once.
///
/// C++ holds every field `const` (`fdmsolverdesc.hpp:36-42`); they are plain
/// public fields here. The struct is cheap to clone because every owned pointer
/// is a [`Shared`].
#[derive(Clone)]
pub struct FdmSolverDesc {
    /// The spatial grid the PDE is discretised on.
    pub mesher: Shared<dyn FdmMesher>,
    /// Boundary conditions applied at every step (empty on the plain European
    /// path).
    pub bc_set: FdmBoundaryConditionSet,
    /// Step conditions applied between steps (empty for European / no
    /// dividends; American, Bermudan and dividend handlers are #636).
    pub condition: Shared<FdmStepConditionComposite>,
    /// The terminal payoff sampled onto the grid at [`maturity`](Self::maturity).
    pub calculator: Shared<dyn FdmInnerValueCalculator>,
    /// Time from today to expiry, in the process's day-counter units.
    pub maturity: Time,
    /// Number of time steps the main scheme takes.
    pub time_steps: Size,
    /// Number of fully-implicit damping steps that precede the main scheme.
    pub damping_steps: Size,
}

impl FdmSolverDesc {
    /// Assembles the descriptor (`fdmsolverdesc.hpp:35-43`).
    pub fn new(
        mesher: Shared<dyn FdmMesher>,
        bc_set: FdmBoundaryConditionSet,
        condition: Shared<FdmStepConditionComposite>,
        calculator: Shared<dyn FdmInnerValueCalculator>,
        maturity: Time,
        time_steps: Size,
        damping_steps: Size,
    ) -> Self {
        FdmSolverDesc {
            mesher,
            bc_set,
            condition,
            calculator,
            maturity,
            time_steps,
            damping_steps,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use crate::instruments::PlainVanillaPayoff;
    use crate::methods::finitedifferences::meshers::{FdmMesherComposite, uniform_1d_mesher};
    use crate::methods::finitedifferences::utilities::FdmCellAveragingInnerValue;
    use crate::option::OptionType;
    use crate::payoff::Payoff;
    use crate::shared::shared;

    #[test]
    fn fields_round_trip_through_new() {
        let mesher_1d = uniform_1d_mesher(0.0, 1.0, 5).unwrap();
        let mesher = shared(FdmMesherComposite::new(vec![mesher_1d]));
        let payoff: Shared<dyn Payoff> = shared(PlainVanillaPayoff::new(OptionType::Call, 100.0));
        let calculator: Shared<dyn FdmInnerValueCalculator> = shared(
            FdmCellAveragingInnerValue::new(payoff, Shared::clone(&mesher) as _, 0),
        );
        let condition = shared(FdmStepConditionComposite::new(&[], Vec::new()));

        let desc = FdmSolverDesc::new(
            Shared::clone(&mesher) as _,
            Vec::new(),
            Shared::clone(&condition),
            Shared::clone(&calculator),
            1.0,
            100,
            0,
        );

        assert_eq!(desc.maturity, 1.0);
        assert_eq!(desc.time_steps, 100);
        assert_eq!(desc.damping_steps, 0);
        assert!(desc.bc_set.is_empty());
        assert!(Shared::ptr_eq(&desc.condition, &condition));
    }
}
