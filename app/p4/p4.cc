#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;
Mutex cout_m;

int bruh(int n)
{
    cout_m.lock();
    cout << "CPU: " << CPU::id() << " bruh - " << n << endl;
    cout_m.unlock();
    Alarm::delay(500000 + (int(Random::random()) % 10000));
    return 0;
}

int poggers(int n)
{
    do {
        cout_m.lock();
        cout << "CPU: " << CPU::id() << " poggers - " << n << endl;
        cout_m.unlock();
        Alarm::delay(1000000 + (int(Random::random()) % 10000));
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main()
{
    cout << "MAIN: Hello world!\n" << endl;

    const int N_THREADS_A = 24; // Aperiodic
    const int N_THREADS_P = 3; // Periodic

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};

    Thread *ts[N_THREADS_A + N_THREADS_P];

    for (int i = 0; i < N_THREADS_P; i++)
    {
        // Thread periodic
        auto conf = Periodic_Thread::Configuration(
            500000 + (int(Random::random()) % 10000),        // periodo
            Periodic_Thread::SAME,          // deadline
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            3
        );
        ts[i] = new Periodic_Thread(conf, &poggers, i);
    }

    for (int i = N_THREADS_P; i < N_THREADS_P + N_THREADS_A; i++)
    {
        // Thread aperiódica
        ts[i] = new Thread(Thread::Configuration(Thread::READY, CRITS[i % 3]), &bruh, i);
    }

    for (int i = 0; i < N_THREADS_A + N_THREADS_P; i++) {
        ts[i]->join();
    }

    cout << "\nMAIN: Bye!" << endl;

    return 0;
}
