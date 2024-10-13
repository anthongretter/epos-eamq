#include <utility/ostream.h>
#include <process.h>

using namespace EPOS;

OStream cout;

int bruh()
{
    cout << "bruh" << endl;
    return 0;
}

int main()
{
    cout << "Hello world!" << endl;

    const int N_THREADS = 5;
    Thread *ts[N_THREADS];

    for (int i = 0; i < N_THREADS; i++)
    {
        // Thread aperiódica
        ts[i] = new Thread(&bruh);
    }

    for (int i = 0; i < N_THREADS; i++)
    {
        // Thread aperiódica
        ts[i]->join();
    }

    return 0;
}

/*
T1 bruh = 0x80800e84
T2 bruh = 0x80800d00
T3 bruh = 0x80800b7c
T4 bruh = 0x808009f8
T5 bruh = 0x80800874

Thread::join(this=0x80800e84,state=1)
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78) MAIN -> IDLE
Thread::dispatch(prev=0x80800e84,next=0x80800d00) T1 -> T2 : bruh
Thread::dispatch(prev=0x80800d00,next=0x808009f8) T2 -> T3 : bruh

F0:
F1:
F2: T5 T4 T3 T2 T1
F3: IDLE MAIN

--------------------------------------------------------


_______________________com handle_______________________
Thread::join(this=0x80800e84,state=1) MAIN espera pelo T1
Thread::dispatch(prev=0xffc3fe1c,next=0x80800e84) MAIN -> T1
bruh - Executa T1
Thread::exit(status=0) [running=0x80800e84] T1 saindo

Thread::dispatch(prev=0x80800e84,next=0x80800b7c) T1 -> T3
bruh - Executa T3
Thread::exit(status=0) [running=0xffc3bc78] IDLE saindo ???

Thread::dispatch(prev=0xffc3bc78,next=0xffc3fe1c) IDLE -> MAIN (??)
Thread::join(this=0x80800d00,state=1)  MAIN esperando T2
Thread::dispatch(prev=0x80800b7c,next=0x808009f8) T3 -> T4
bruh - Executa T4
Thread::exit(status=0) [running=0x808009f8]  T4 saiu

Thread::dispatch(prev=0x808009f8,next=0x80800d00) T4 -> T2
bruh - Executa T2
Thread::exit(status=0) [running=0x80800d00] T2 saiu

Thread::dispatch(prev=0x80800d00,next=0x80800b7c) T2 -> T3
Thread::join(this=0x80800b7c,state=0) T3 saindo

Assertion fail: running() != this, function=int EPOS::S::Thread::join(), fileh
read.cc, line=130

*/

/*
_______________________sem handle_______________________
Thread::join(this=0x80800e84,state=1) MAIN espera por T1
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78) MAIN -> IDLE
Thread::idle(this=0x80800e84) LEAVE IDLE, troca fila ate prox (fila 2)
Thread::dispatch(prev=0x80800e84,next=0x80800d00) T1 -> T2
bruh - T2 executou
Thread::exit(status=0) [running=0x80800d00] T2 sai

Thread::dispatch(prev=0x80800d00,next=0x808009f8) T2 -> T4
bruh - T4 executou
Thread::exit(status=0) [running=0x808009f8] T4 sai

Thread::dispatch(prev=0x808009f8,next=0x80800e84) T4 -> T1
Thread::dispatch(prev=0x80800e84,next=0x80800b7c) T1 -> T3
bruh - T3 executou
Thread::exit(status=0) [running=0x80800b7c] T3 sai

Thread::dispatch(prev=0x80800b7c,next=0x80800e84) T3 -> T1
Thread::dispatch(prev=0x80800e84,next=0x80800874) T1 -> T5
bruh - T5 executou
Thread::exit(status=0) [running=0x80800874] T5 saiu

Thread::dispatch(prev=0x80800874,next=0x80800e84) T5 -> T1
*/
