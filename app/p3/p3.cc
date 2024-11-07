#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

int bruh(int n)
{
    cout << "CPU: " << CPU::id() << " bruh - " << n << endl;
    //Alarm::delay(50000);
    return 0;
}

int poggers(int n)
{
    do {
        cout << "CPU: " << CPU::id() << " poggers - " << n << endl;
        //Alarm::delay(50000);
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main()
{
    cout << "MAIN: Hello world!" << endl;

    const int N_THREADS_A = 6; // Aperiodic
    const int N_THREADS_P = 3; // Periodic

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};


    Thread *ts[N_THREADS_A + N_THREADS_P];

    for (int i = 0; i < N_THREADS_A; i++)
    {
        // Thread aperiódica
        ts[i] = new Thread(Thread::Configuration(Thread::READY, CRITS[i % 3]), &bruh, i);
    }

    for (int i = N_THREADS_A; i < N_THREADS_P + N_THREADS_A; i++)
    {
        // Thread periodic
        auto conf = Periodic_Thread::Configuration(
            500000 + (int(Random::random()) % 1000000),        // periodo
            120000 + (int(Random::random()) % 2400000),          // deadline
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            2
        );
        ts[i] = new Periodic_Thread(conf, &poggers, i);
    }

    for (int i = 0; i < N_THREADS_A + N_THREADS_P; i++) {
        ts[i]->join();
    }

    return 0;
}
