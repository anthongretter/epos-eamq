#ifndef __traits_h
#define __traits_h

#include <system/config.h>

__BEGIN_SYS

// Build
template <>
struct Traits<Build> : public Traits_Tokens
{
    // Basic configuration
    static const unsigned int SMOD = LIBRARY;
    static const unsigned int ARCHITECTURE = IA32;
    static const unsigned int MACHINE = PC;
    static const unsigned int MODEL = Legacy_PC;
    static const unsigned int CPUS = 4;
    static const unsigned int NETWORKING = STANDALONE;
    static const unsigned int EXPECTED_SIMULATION_TIME = 60; // s (0 => not simulated)

    // Default flags
    static const bool enabled = true;
    static const bool monitored = true;
    static const bool debugged = true;
    static const bool hysterically_debugged = false;
};

// Utilities
template <>
struct Traits<Debug> : public Traits<Build>
{
    static const bool error = true;
    static const bool warning = true;
    static const bool info = false;
    static const bool trace = false;
};

template <>
struct Traits<AAA> : public Traits<Build>
{
    static const bool debugged = true;
};

template <>
struct Traits<PEAMQ> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<EAMQ> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<GEAMQ> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Lists> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Spin> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Heaps> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Observers> : public Traits<Build>
{
    // Some observed objects are created before initializing the Display
    // Enabling debug may cause trouble in some Machines
    static const bool debugged = false;
};

// System Parts (mostly to fine control debugging)
template <>
struct Traits<Boot> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Setup> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Init> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Framework> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Aspect> : public Traits<Build>
{
    static const bool debugged = hysterically_debugged;
};

__END_SYS

// Mediators
#include __ARCHITECTURE_TRAITS_H
#include __MACHINE_TRAITS_H

__BEGIN_SYS

// API Components
template <>
struct Traits<Application> : public Traits<Build>
{
    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = Traits<Machine>::HEAP_SIZE;
    static const unsigned int MAX_THREADS = Traits<Machine>::MAX_THREADS;
};

template <>
struct Traits<System> : public Traits<Build>
{
    static const bool multithread = (Traits<Application>::MAX_THREADS > 1) || (CPUS > 1);
    static const bool multicore = multithread && (CPUS > 1);
    static const bool multiheap = false;

    static const unsigned long LIFE_SPAN = 1 * YEAR; // s
    static const unsigned int DUTY_CYCLE = 1000000;  // ppm

    static const bool reboot = true;

    static const unsigned int STACK_SIZE = Traits<Machine>::STACK_SIZE;
    static const unsigned int HEAP_SIZE = (Traits<Application>::MAX_THREADS + Traits<Build>::CPUS) * Traits<Application>::STACK_SIZE;

    static const unsigned int RUN_TO_HALT = false;

};

template <>
struct Traits<Thread> : public Traits<Build>
{
    static const bool enabled = Traits<System>::multithread;
    static const bool smp = Traits<System>::multicore;
    static const bool trace_idle = hysterically_debugged;
    static const bool simulate_capacity = false;
    static const int priority_inversion_protocol = NONE;


    typedef IF<(CPUS > 1), PEAMQ, EAMQ>::Result Criterion;
    static const unsigned int QUANTUM = 10000; // us

    static const bool debugged = false;
};

template <>
struct Traits<Scheduler<Thread>> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Synchronizer> : public Traits<Build>
{
    static const bool enabled = Traits<System>::multithread;
    static const bool debugged = false;
};

template <>
struct Traits<Alarm> : public Traits<Build>
{
    static const bool visible = hysterically_debugged;
    static const bool debugged = false;
};

template <>
struct Traits<Address_Space> : public Traits<Build>
{
    static const bool debugged = false;
};

template <>
struct Traits<Segment> : public Traits<Build>
{
    static const bool debugged = false;
};

__END_SYS

#endif