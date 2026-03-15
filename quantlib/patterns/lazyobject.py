"""LazyObject — calculation-on-demand with caching.

LazyObject inherits both Observable and Observer. It caches calculation
results and only recomputes when dependencies change. Subclasses override
performCalculations() with their computation logic.
"""

from __future__ import annotations

from abc import abstractmethod

from quantlib.patterns.observable import Observable, Observer
from quantlib.patterns.singleton import Singleton


class LazyObject(Observable, Observer):
    """Framework for objects that perform expensive calculations with caching.

    Key behavior:
    - calculate() triggers performCalculations() only if needed
    - freeze()/unfreeze() lock/unlock cached values
    - Notification forwarding is configurable per-instance
    - Recursion guard prevents infinite update loops
    """

    class Defaults(Singleton):
        """Per-session defaults for LazyObject notification forwarding."""

        def __init__(self):
            self._forwards_all = True

        def forwardsAllNotifications(self) -> bool:
            """Return True if new LazyObjects should forward all notifications."""
            return self._forwards_all

        def forwardFirstNotificationOnly(self) -> None:
            """Set default to forward only first notification."""
            self._forwards_all = False

        def alwaysForwardNotifications(self) -> None:
            """Set default to forward all notifications."""
            self._forwards_all = True

    def __init__(self):
        Observable.__init__(self)
        Observer.__init__(self)
        self._calculated = False
        self._frozen = False
        self._always_forward = LazyObject.Defaults.instance().forwardsAllNotifications()
        self._updating = False

    def calculate(self) -> None:
        """Perform calculation if not already cached and not frozen.

        Sets _calculated = True BEFORE performCalculations() to prevent
        infinite recursion during bootstrapping. Resets on exception.
        """
        if not self._calculated and not self._frozen:
            self._calculated = True
            try:
                self.performCalculations()
            except Exception:
                self._calculated = False
                raise

    @abstractmethod
    def performCalculations(self) -> None:
        """Override in subclasses with actual computation logic."""

    def recalculate(self) -> None:
        """Force immediate recalculation.

        Saves frozen state, temporarily clears both flags, calls calculate(),
        restores frozen, then notifies. On exception: restores frozen,
        notifies, then re-raises.
        """
        was_frozen = self._frozen
        self._calculated = False
        self._frozen = False
        try:
            self.calculate()
        except Exception:
            self._frozen = was_frozen
            self.notifyObservers()
            raise
        self._frozen = was_frozen
        self.notifyObservers()

    def freeze(self) -> None:
        """Lock cached values — prevent recalculation on updates."""
        self._frozen = True

    def unfreeze(self) -> None:
        """Allow recalculation again.

        Only calls notifyObservers() if was actually frozen.
        """
        if self._frozen:
            self._frozen = False
            self.notifyObservers()

    def isCalculated(self) -> bool:
        """Return True if results are currently cached."""
        return self._calculated

    def setCalculated(self, calculated: bool) -> None:
        """Manually set the calculation status flag."""
        self._calculated = calculated

    def update(self) -> None:
        """Observer callback — mark as uncalculated, notify dependents.

        Uses _updating flag as recursion guard to break cycles.
        Forwards notification if calculated or alwaysForward.
        """
        if self._updating:
            return

        self._updating = True
        try:
            if self._calculated or self._always_forward:
                self._calculated = False
                if not self._frozen:
                    self.notifyObservers()
        finally:
            self._updating = False

    def forwardFirstNotificationOnly(self) -> None:
        """Forward only the first notification after a calculation."""
        self._always_forward = False

    def alwaysForwardNotifications(self) -> None:
        """Forward all change notifications."""
        self._always_forward = True
