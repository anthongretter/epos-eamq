#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

const int N_THREADS_A = 64; // Aperiodic

int isPrime(const int n) {
  bool is_prime = true;

  // 0 and 1 are not prime numbers
  if (n == 0 || n == 1) {
    is_prime = false;
  }

  // loop to check if n is prime
  for (int i = 2; i <= n/2; ++i) {
    if (n % i == 0) {
      is_prime = false;
      break;
    }
  }

  if (is_prime)
    cout << " found a prime number " << n << endl;

  return 0;
}

int work(int n)
{
    cout << "WORK: Hello world! " << n << endl;
    for (int i = 0; i < 1000; i++)
    {
        isPrime(i);
    }

    cout << "WORK: Bye world! " << n << endl;
    return 0;
}


int main()
{
    cout << "MAIN: Hello world!" << endl;

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};


    Thread *ts[N_THREADS_A];

    for (int i = 0; i < N_THREADS_A; i++)
    {
        // Thread aperiódica
        ts[i] = new Thread(Thread::Configuration(Thread::READY, CRITS[i % 3]), &work, i);
    }

    for (int i = 0; i < N_THREADS_A; i++) {
        ts[i]->join();
    }
    cout << "System is going down!" << endl;

    return 0;
}
