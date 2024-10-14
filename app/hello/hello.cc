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

    const int N_THREADS = 2;
    Thread *ts[N_THREADS];

    for (int i = 0; i < N_THREADS; i++)
    {
        // Thread aperiódica
        ts[i] = new Thread(&bruh);
    }

    // ts[N_THREADS - 1] = new Thread(Thread::Configuration(Thread::READY, Thread::LOW), &bruh);

    for (int i = 0; i < N_THREADS; i++) {
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

------------------- ULTIMO TESTE --------------------------
Task(entry=0x80000038) => 0xffc3ffa0
Scheduling_List::insert(e=0xffc3fee0) => {p=0x00000000,o=0xffc3fe1c,n=0x000000
}
Thread(entry=0x80000038,state=0,priority=-1,queue=3,stack={b=0xffc3be14,s=163}
,context={b=0xffc3fde4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x000000,
sp=0xffb03c28,ip=0x80000038,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3x
3fffc000}}) => 0xffc3fe1c
Task::enroll(t=0, o=0xffc3fe1c)

Scheduling_List::insert(e=0xffc3bd3c) => {p=0x00000000,o=0xffc3bc78,n=0x000000
}
Thread(entry=0x80003ad6,state=1,priority=2147483647,queue=3,stack={b=0xffc37c,
s=16384},context={b=0xffc3bc40,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0
0000000,sp=0xffb03c28,ip=0x80003ad6,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,cs1
0,cr3=0x3fffc000}}) => 0xffc3bc78
Init_Application()
Heap(addr=0x80401000,bytes=4194304) => 0x80400040
Init_End()
Hello world!
Scheduling_List::insert(e=0x80800f48) => {p=0x00000000,o=0x80800e84,n=0x000000
}
Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc33c5s
=16384},context={b=0xffc37c20,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css0
,cr3=0x3fffc000}}) => 0x80800e84
Task::enroll(t=0, o=0x80800e84)
Scheduling_List::insert(e=0x80800dc4) => {p=0x00000000,o=0x80800d00,n=0x000000
}
Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc2fc3s
=16384},context={b=0xffc33c00,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css0
,cr3=0x3fffc000}}) => 0x80800d00
Task::enroll(t=0, o=0x80800d00)
Scheduling_List::insert(e=0x80800c40) => {p=0x00000000,o=0x80800b7c,n=0x000000
}
Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc2bc1s
=16384},context={b=0xffc2fbe0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css0
,cr3=0x3fffc000}}) => 0x80800b7c
Task::enroll(t=0, o=0x80800b7c)
Scheduling_List::insert(e=0x80800abc) => {p=0x00000000,o=0x808009f8,n=0x000000
}
Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc27bfs
=16384},context={b=0xffc2bbc0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css0
,cr3=0x3fffc000}}) => 0x808009f8
Task::enroll(t=0, o=0x808009f8)
Scheduling_List::insert(e=0x80800938) => {p=0x00000000,o=0x80800874,n=0x000000
}
Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc23bds
=16384},context={b=0xffc27ba0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css0
,cr3=0x3fffc000}}) => 0x80800874
Task::enroll(t=0, o=0x80800874)
Thread::join(this=0x80800e84,state=1)
saiu da fila: 3
0 - 0
1 - 0
2 - 4
3 - 0
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78)
Thread::idle(this=0x80800e84)
saiu da fila: 2
0 - 0
1 - 0
2 - 4
3 - 0
Thread::dispatch(prev=0x80800e84,next=0x80800d00)
bruh
Thread::exit(status=0) [running=0x80800d00]
saiu da fila: 2
0 - 0
1 - 0
2 - 3
3 - 0
Thread::dispatch(prev=0x80800d00,next=0x808009f8)
bruh
Thread::exit(status=0) [running=0x808009f8]
saiu da fila: 2
0 - 0
1 - 0
2 - 2
3 - 0
Thread::dispatch(prev=0x808009f8,next=0x80800e84)
saiu da fila: 2
0 - 0
1 - 0
2 - 2
3 - 0
Thread::dispatch(prev=0x80800e84,next=0x80800b7c)
bruh
Thread::exit(status=0) [running=0x80800b7c]
saiu da fila: 2
0 - 0
1 - 0
2 - 1
3 - 0
Thread::dispatch(prev=0x80800b7c,next=0x80800e84)
saiu da fila: 2
0 - 0
1 - 0
2 - 1
3 - 0
Thread::dispatch(prev=0x80800e84,next=0x80800874)
bruh
Thread::exit(status=0) [running=0x80800874]
saiu da fila: 2
0 - 0
1 - 0
2 - 0
3 - 0
Thread::dispatch(prev=0x80800874,next=0x80800e84)


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
