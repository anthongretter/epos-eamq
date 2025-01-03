// EPOS Semaphore Implementation

#include <synchronizer.h>

__BEGIN_SYS

Semaphore::Semaphore(long v) : _value(v)
{
    db<Synchronizer>(TRC) << "Semaphore(value=" << _value << ") => " << this << endl;
}


Semaphore::~Semaphore()
{
    db<Synchronizer>(TRC) << "~Semaphore(this=" << this << ")" << endl;
}


void Semaphore::p()
{

    begin_atomic();
    db<Thread>(TRC) << "Semaphore::p(this=" << this << ",value=" << _value << ")" << endl;
    if(fdec(_value) < 1)
        sleep();
    end_atomic();
}


void Semaphore::v()
{
    db<Thread>(TRC) << "Semaphore::v(this=" << this << ",value=" << _value << ")" << endl;

    begin_atomic();
    if(finc(_value) < 0)
        wakeup();
    end_atomic();
}

__END_SYS
