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
    int wt = 3000000 + (unsigned(Random::random()) % 5000000);      // 3s - 8s
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

        const int size = 100;
        int arr[size];

        // grandes pulos de acesso para causar cache miss
        for (int i = 0; i < size; i += (unsigned(Random::random()) % 10)) {
            // esconde surpresa
            arr[i] = 42;
        }

        cout_m.lock();
        cout << "CPU: " << CPU::id() << " poggers - " << n << " ACHOU: " << arr[unsigned(Random::random()) % size] << endl;
        cout_m.unlock();
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main()
{
    cout << "MAIN: Hello world!\n" << endl;

    const int N_THREADS_A = 12; // Aperiodic
    const int N_THREADS_P = 4; // Periodic

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};

    Thread *ts[N_THREADS_A + N_THREADS_P];

    for (int i = 0; i < N_THREADS_P; i++)
    {
        // Thread periodic
        auto conf = Periodic_Thread::Configuration(
            500000 + (unsigned(Random::random()) % 500000),            //   0.5s - 1s
            (500000 + (unsigned(Random::random()) % 500000)) * 4,
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            2
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
