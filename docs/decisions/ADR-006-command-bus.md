# ADR-006: All state changes go through a CommandBus

## Status
Accepted (Phase 0)

## Context
A scientific exploration tool needs undo/redo, a record of user
actions, and the ability for any future agent (or script) to drive
the same actions a human can. Direct widget mutation makes all
three impossible.

## Decision
Every state-changing operation is a `Command` object with `do()`
and `undo()` methods. Widgets, scripts, and (future) agents submit
commands to a single `CommandBus`. The bus executes, records, and
emits change events.

## Consequences
- + Free undo/redo
- + Reproducible action log per session
- + Same surface for human and future agent
- - Slightly more boilerplate per feature
- - Requires discipline; enforced by code review

## Alternatives considered
- Direct widget mutation: no undo, no replay
- Plain reducer pattern: works but less ergonomic for undo
