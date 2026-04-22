# Fiber Job System

A minimal cooperative scheduling layer intended to sit above the `job_system` project.

**Status:** portable stackful context switching implemented with `boost::context`

## What is a fiber job system?

A fiber job system lets a task pause itself and continue later from the exact same point, without blocking the thread running it.

Think of it like a bookmark:

- A thread is the reader  
- A fiber is the task  
- yield places a bookmark  
- resume continues from that bookmark  

When a fiber pauses, the thread is free to run something else.

### How it works

1. Submit fibers A and B  
2. Worker runs A  
   A does some work  
   A pauses (yield)  
3. Worker runs B while A is paused  
4. Scheduler resumes paused fibers  
5. A continues exactly where it paused  

### Visual timeline

Worker:   run A --- yield --- run B --- resume A  
Fiber A:  start --- work --- pause --- continue --- finish  

### In this project

- Submitted work becomes fibers  
- A worker executes one fiber at a time  
- yield_current() pauses the current fiber  
- resume_all() makes paused fibers runnable again  
- Resumed fibers continue exactly where they paused  

### Key idea

A fiber is not restarted.  
It continues from the exact point where it yielded.

## Purpose

This project explores cooperative scheduling on top of the existing execution stack.

## Notes

- Uses Boost.Context for portable stackful context switching.
- Yielded fibers currently keep worker affinity after first execution.
- Still intentionally minimal.
- Manual suspension and resumption are supported via yield_current() and resume_all().
- Structured waiting on other jobs, counters, or events is not integrated yet.
