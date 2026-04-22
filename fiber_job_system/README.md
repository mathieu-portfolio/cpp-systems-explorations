# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** simulated scheduler with `FiberTask` state ownership implemented, real context switching not implemented

## What is a fiber job system?

A fiber job system allows execution to yield and later resume without blocking an OS worker thread.

The current version models that behavior with scheduler-owned tasks:

- submitted work becomes scheduler-owned runnable fibers
- one-shot jobs can still be submitted with `submit()`
- resumable work can be submitted as a `FiberTask`
- the task object owns its logical progress
- suspended fibers are later moved back to runnable work with `resume_all()`

This version does not yet preserve native stack context, but logical resume state now belongs to the fiber task itself rather than external captured variables.

## Purpose

This project explores cooperative scheduling semantics on top of the existing execution stack before introducing real context-switching machinery.

It is intended to answer:

- how runnable and suspended work are represented
- how yielding changes scheduler state
- how fiber-owned state differs from captured-lambda state

## Current Model

The current implementation supports three submission forms:

- one-shot jobs:
  - submitted with `submit()`
  - run once to completion

- lambda-based resumable jobs:
  - submitted with `submit_resumable()`
  - adapted internally into a `FiberTask`

- task-based resumable jobs:
  - submitted with `submit_task()`
  - store logical progress directly inside the task object

## Planned Public Model

The current API surface is:

- `submit()` to create one-shot schedulable work
- `submit_resumable()` as a convenience wrapper for step-based resumable work
- `submit_task()` to submit a stateful `FiberTask`
- `yield_current()` to cooperatively suspend the current one-shot fiber
- `resume_all()` to move suspended fibers back to runnable work

## Notes

This is a learning-focused implementation. The goal is to validate scheduler ownership and task-owned resume semantics before introducing real fiber context switching.
