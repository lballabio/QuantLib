"""Observer/Observable pattern for change notification.

This module implements the non-thread-safe version of QuantLib's
Observer pattern. Observable objects notify registered observers
when they change. ObservableSettings provides global control
over notification behavior (disable, defer).
"""

from __future__ import annotations

import weakref
from abc import ABC, abstractmethod

from quantlib.patterns.singleton import Singleton


class ObservableSettings(Singleton):
    """Global settings for the Observer pattern.

    Controls whether notifications are enabled, disabled, or deferred.
    When deferred, observer updates are queued and replayed on enableUpdates().
    """

    def __init__(self):
        self._updates_enabled = True
        self._updates_deferred = False
        self._deferred_observers: dict[Observer, bool] = {}
        self._running_deferred_updates = False

    def disableUpdates(self, deferred: bool = False) -> None:
        """Disable notifications. If deferred=True, queue them for later."""
        self._updates_enabled = False
        self._updates_deferred = deferred

    def enableUpdates(self) -> None:
        """Re-enable notifications. Call update() on all deferred observers.

        If any update() raises, continues processing remaining observers,
        then raises after all are done.
        """
        self._updates_enabled = True
        self._updates_deferred = False

        if self._deferred_observers:
            self._running_deferred_updates = True
            error = None
            try:
                for observer, active in list(self._deferred_observers.items()):
                    if active:
                        try:
                            observer.update()
                        except Exception as e:
                            if error is None:
                                error = e
            finally:
                self._deferred_observers.clear()
                self._running_deferred_updates = False

            if error is not None:
                raise error

    def updatesEnabled(self) -> bool:
        """Return True if updates are currently enabled."""
        return self._updates_enabled

    def updatesDeferred(self) -> bool:
        """Return True if updates are being deferred."""
        return self._updates_deferred

    def runningDeferredUpdates(self) -> bool:
        """Return True if currently processing deferred updates."""
        return self._running_deferred_updates

    def registerDeferredObservers(self, observers: set[Observer]) -> None:
        """Copy observers into the deferred set."""
        if self._updates_deferred:
            for obs in observers:
                self._deferred_observers.setdefault(obs, True)

    def unregisterDeferredObserver(self, observer: Observer) -> None:
        """Remove observer from the deferred set.

        During deferral: erase from dict.
        During deferred replay: mark as inactive (skip during iteration).
        """
        if self._updates_deferred:
            self._deferred_observers.pop(observer, None)
        else:
            if observer in self._deferred_observers:
                self._deferred_observers[observer] = False


class Observable:
    """Base class for objects that notify observers of changes.

    Maintains weak references to observers via a dict mapping id(observer)
    to weakref.ref. When notifyObservers() is called, all live registered
    observers have their update() method called. Dead observers are
    automatically cleaned up via weakref callbacks.
    """

    def __init__(self):
        self._observers: dict[int, weakref.ref] = {}

    def _make_weak_callback(self, observer_id: int):
        """Create a weakref callback that removes the dead observer."""

        def callback(ref):
            self._observers.pop(observer_id, None)

        return callback

    def registerObserver(self, observer: Observer) -> None:
        """Register an observer (stored as weak reference)."""
        oid = id(observer)
        if oid not in self._observers:
            self._observers[oid] = weakref.ref(observer, self._make_weak_callback(oid))

    def unregisterObserver(self, observer: Observer) -> None:
        """Unregister an observer."""
        settings = ObservableSettings.instance()
        if settings.updatesDeferred() or settings.runningDeferredUpdates():
            settings.unregisterDeferredObserver(observer)
        self._observers.pop(id(observer), None)

    def _get_live_observers(self) -> list[Observer]:
        """Dereference all weak refs, returning only live observers."""
        live = []
        for ref in list(self._observers.values()):
            obs = ref()
            if obs is not None:
                live.append(obs)
        return live

    def notifyObservers(self) -> None:
        """Notify all registered observers by calling their update() method.

        If any observer's update() raises, continues processing remaining
        observers, then raises the first exception after all are done.
        """
        settings = ObservableSettings.instance()

        if not settings.updatesEnabled():
            live = self._get_live_observers()
            settings.registerDeferredObservers(set(live))
            return

        error = None
        for observer in self._get_live_observers():
            try:
                observer.update()
            except Exception as e:
                if error is None:
                    error = e
        if error is not None:
            raise error

    def __copy__(self):
        """Copy creates a new Observable with no observers."""
        new = self.__class__.__new__(self.__class__)
        new._observers = {}
        return new

    def __deepcopy__(self, memo):
        """Deep copy creates a new Observable with no observers."""
        new = self.__class__.__new__(self.__class__)
        memo[id(self)] = new
        new._observers = {}
        return new


class Observer(ABC):
    """Abstract base for objects that react to Observable changes.

    Subclasses must implement update(). Use registerWith() to subscribe
    to an Observable's notifications.
    """

    def __init__(self):
        self._observables: set[Observable] = set()

    @abstractmethod
    def update(self) -> None:
        """Called when an observed Observable changes. Must be overridden."""

    def registerWith(self, observable: Observable | None) -> None:
        """Start observing the given Observable. No-op if None."""
        if observable is None:
            return
        observable.registerObserver(self)
        self._observables.add(observable)

    def registerWithObservables(self, observer: Observer) -> None:
        """Observe everything that another Observer observes."""
        for observable in observer._observables:
            self.registerWith(observable)

    def unregisterWith(self, observable: Observable | None) -> None:
        """Stop observing the given Observable."""
        if observable is None:
            return
        observable.unregisterObserver(self)
        self._observables.discard(observable)

    def unregisterWithAll(self) -> None:
        """Stop observing all Observables."""
        for observable in list(self._observables):
            observable.unregisterObserver(self)
        self._observables.clear()

    def deepUpdate(self) -> None:
        """Force update, propagating through the notification chain.

        Default implementation calls update(). Override in subclasses
        that need to propagate through nested observers.
        """
        self.update()
