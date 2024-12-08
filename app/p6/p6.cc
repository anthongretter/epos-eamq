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
    int wt = 1000000 + (unsigned(Random::random()) % 2000000);      // 1s - 3s
    cout_m.lock();
    cout << "CPU: " << CPU::id() << " bruh - " << n << ": vou esperar " << wt / 1000000  << "s" << endl;
    cout_m.unlock();
    Alarm::delay(wt);
    cout_m.lock();
    cout << "CPU: " << CPU::id() << " bruh - " << n << ": bye!" << endl;
    cout_m.unlock();
    return 0;
}

int poggers(int n)
{
    do {
        cout_m.lock();
        cout << "CPU: " << CPU::id() << " poggers - " << n << endl;
        cout_m.unlock();
        const int size = 1000000;  // Adjust size as needed
        volatile int arr[size];

        // Access memory with a large stride to cause cache misses
        for (int i = 0; i < size; i += (unsigned(Random::random()) % 10)) {  // random stride
            arr[i] = (unsigned(Random::random()) % size);
        }
        volatile int aleatorio = arr[(unsigned(Random::random()) % size)];
//        Alarm::delay(100000 + (unsigned(Random::random()) % 10000));     // 0.1s - 0.2s
        cout_m.lock();
        cout << "CPU: " << CPU::id() << " poggers - " << n << ": oq achei: " << aleatorio << ": bye!" << endl;
        cout_m.unlock();
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main()
{
    cout << "MAIN: Hello world!\n" << endl;

    const int N_THREADS_A = 0; // Aperiodic
    const int N_THREADS_P = 1; // Periodic

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};

    Thread *ts[N_THREADS_A + N_THREADS_P];

    for (int i = 0; i < N_THREADS_P; i++)
    {
        // Thread periodic
        auto conf = Periodic_Thread::Configuration(
            500000 + (unsigned(Random::random()) % 500000),            //   0.5s - 1s
            (500000 + (unsigned(Random::random()) % 500000)) * 1000,     //   500s - 1000s
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            10
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
