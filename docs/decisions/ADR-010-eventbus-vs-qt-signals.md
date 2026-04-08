# ADR-010: EventBus vs Qt signals/slots — when to use which

## Status
Accepted (Phase 1)

## Context
Qt provides a built-in signal/slot system. ADR-006 calls for a
CommandBus. We also need decoupled cross-module communication (e.g.,
"a document was opened" needs to reach UI, future plot engine, and
future status bar — none of which should know about each other).

The question is: when do we use direct Qt signals/slots, and when
do we route through a central EventBus?

## Decision
Two-tier system:

### Direct Qt signals/slots
Use for **local, tightly-coupled** communication within a module:
- Widget → Widget (button click → dialog open)
- Parent → Child within the same dock
- QAbstractItemModel signals (dataChanged, etc.)
- Any case where sender and receiver are in the same module and
  have a direct pointer to each other

### EventBus
Use for **cross-module, decoupled** communication:
- Document lifecycle: opened, closed, modified
- Selection changes (global selection state)
- Theme changes
- Any event where the sender should not know who listens

EventBus is implemented as a `QObject` singleton with typed event
enums and `QVariant` payloads. Subscribers connect to specific
event types. Under the hood it uses Qt signals, but the API hides
the signal/slot boilerplate and enforces the event type contract.

### CommandBus (Phase 3+)
For **state-changing operations** that need undo/redo. Deferred to
Phase 3 because Phase 1 is read-only (no data mutation).

## Consequences
- + Clear decision boundary: local → Qt signals, cross-module →
  EventBus, state-change → CommandBus
- + EventBus provides a single point for logging all cross-module
  events (useful for debugging and future replay)
- + Modules stay decoupled: data/ does not import ui/
- - Two systems to learn (but both are Qt signals under the hood)
- - EventBus adds a level of indirection

## Alternatives considered
- **Qt signals only**: works but creates tight coupling; data/
  module would need to know about UI types to connect signals.
- **EventBus only (no direct signals)**: too heavy for local
  widget-to-widget communication; defeats the purpose of Qt's
  built-in system.
- **Third-party event library (e.g., eventpp)**: unnecessary
  dependency; Qt's signal system is sufficient as a transport.
