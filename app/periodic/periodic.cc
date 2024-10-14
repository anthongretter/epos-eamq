#include <utility/ostream.h>
#include <real-time.h>
#include <time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

int bruh(int n)
{
    do {
        cout << "bruh " << n << endl;
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main()
{
    cout << "Hello world!" << endl;

    const int N_THREADS = 3;
    const Microsecond BASE = 100000;
    Thread *ts[N_THREADS];

    for (int i = 0; i < N_THREADS; i++)
    {
        auto conf = Periodic_Thread::Configuration(
            BASE,
            Periodic_Thread::SAME,
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            2
        );
        ts[i] = new Periodic_Thread(conf, &bruh, i);
    }

    for (int i = 0; i < N_THREADS; i++) {
        ts[i]->join();
    }

    return 0;
}
