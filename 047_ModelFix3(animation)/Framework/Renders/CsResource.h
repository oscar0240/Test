#pragma once

class CsResource
{
public:
	static void CreateRawBuffer(UINT byteWidth, void* initData, ID3D11Buffer** buffer);
	static void CreateStructuredBuffer(UINT stride, UINT count, void* initData, ID3D11Buffer** buffer);

	static void CreateSrv(ID3D11Buffer* buffer, ID3D11ShaderResourceView** srv);
	static void CreateUav(ID3D11Buffer* buffer, ID3D11UnorderedAccessView** uav);

	static ID3D11Buffer* CreateAndCopyBuffer(ID3D11Buffer* src);
};

/*
Thread 기초
기본개념 - 윈도우 운영체제를 제외한 대부분의 운영체제에서 프로세스(CPU시간을 할당 받을 수 잇는 상태)는 
CPU시간을 할당받아 실행 중인 프로그램을 일컫는다. 프로그램이 저장장치에 파일로 존재하는 정적인 개념인데 반해,
프로세스는 코드(CPU명령), 데이터(전역변수, 정적변수), 리소스(파일소켓,그림파일, 사운드 파일 + 할당된 메모리 상태 등)를
파일에서 읽어들여 작업을 수행하는 동적인 개념이다.

윈도우 응용프로그램이 CPU 시간을 할당받아(쓰레드가 기본단위)실행하려면 스레드가 최소 하나 이상이 필요하다. 
응용 프로그램 실행시 최초로 실행되는 스레드를 주 스레드(primary thread) 또는 메인 스레드(main thread) 라 부르는다.
WinMain() 또는 main() 함수에서 실행한다. 만약 응용프로그램에서 주 스레드와 별도로 동시에 수행하고자 하는 
작업이 있다면 스레드를 추가로 생성해 이 스레드가 해당 작업을 수행하게 하면 된다. 
많은 윈도우 응용프로그램이 이렇게 구현되는데 이를 멀티 응용프로그램(multithread application)이라 한다.

요약하자면 Thread는 프로세스 내에서 실제로 작업을 수행하는 주체를 의미합니다. 두개 이상의 Thread가 실행되면
멀티쓰레드라고 부른다. CPU 하나가 스레드 두개를 동시에 실행할 수는 없지만(single core CPU) 교대로 실행하는 일은 가능하다.
교대로 실행하는 간격이 충분히 짧다면 사용자는 두 스레드가 동시에 실행되는 것처럼 느낀다. 이게 가능한게 쓰레드는 병렬로 실행됨

ex) A,B라는 쓰레드가 교차로 실행이 되고있는데 A쓰레드에서 어떠한 변수를 가지고 실행을 하고 있는데 
   실행이 끝나기 전에 B쓰레드가 실행이 되면서 A에서 쓰던 변수를 쓰려고 하면 문제가 생긴다. 
   그래서 하나의 쓰레드만 들어갈 수 있는 임계영역(CriticalSection 변수를 묶어주는 영역)이라는 
   것을 만들고 어떤 쓰레드가 임계영역에 들어가서 LOCK을 걸고 작업을 수행하고 LOCK을 풀고 나오면 다른 쓰레드가 들어간다.

@주의 사항 mutex.lock을 걸고 unlock으로 풀어주지 않으면 다음 쓰레드는 영원히 실행안돼고 대기를 한다
          이를 DeadLock(컴퓨터가 다운)이라고 한다.

임계역역이 1개 - mutex
임계영역이 여러개 - 세마포어(Semaphore)

mutex.lock
실행할 함수 내용
mutex.unlock
이 사이를 CriticalSection(임계영역)이라고 한다.

context switching - 쓰레드 하나의 작업을 진행하기 위해 해당 쓰레드의 context를 읽어오고 다시 다른 쓰레드 작업을 변경할 때
                    이전 쓰레드의 context를 저장하고 다음 작업을 진행할 다음쓰레드의 context를 읽어오는 작업.

					즉. 한 쓰레드에서 다른 쓰레드로 넘기는 과정

                    ex) 윈도우에서 많은 프로그램을 실행시킬 시에 느려지게 되는데 작업해야 할 쓰레드가 많아져서
					컨텍스트 스위칭이 매우 빈번하게 발생해서 그런것이다.
					여러개의 쓰레드가 교차로 실행되는데 쓰레드 하나가 처리되는 시간보다 교체가 되는 시간이 더 짧아서
                    교체가 너무 빈번히 일어나는 경우
*/

/*
[numthread(x,y,z)]
CS()
{ 
}                 
이런식으로 만드는데 numtread의 x가 256이면 CS()함수가 256개로 나뉜다. 
Shader->Dispatch(x,y,z)로 이 함수를 실행시킨다. Dispatch의 x를 2로 하면
256개의 쓰레드가 2개의 그룹으로 나뉜다. 한 그룹내에서 쓰레드끼리의 메모리 공유는
가능하지만 2개로 나뉜 그룹끼리는 메모리 공유가 안됀다.


Compute Shader 기초
 GPU가 좋아지면서 CPU를 능가하는 병렬처리 연산능력을 가지게 되었다. 이 병령 처리능력을
랜더링 이외에도 범용적으로 연산능력을 활용할 수 있도록 한 것.

CS를 사용한 연산의 이점(PS의 연산과 비교)
1.출력할 곳의 리소스를 임의의 위치로 쓸수 있음
2.데이터 공유, Thread 동기화의 메커니즘 설계
3.지정한 수의 Thread를 명시적으로 구동시켜, 퍼포먼스를 최적화 할 수 있음
4.렌더링 파이프라인과 상관없으므로 코드의 유지보수가 간단
5.쓰레드를 사용해서 병렬적으로 처리된다

특징
1.GPU 랜더링 파이프라인과는 동떨어진 독립된 쉐이더 그래픽 랜더링과 관계없이 사용 가능
2.같은 GroupId에 속하는 Thread간에는 스레드 그룹 공유메모리의 내용을 공유할 수 있다.
  다른 그룹끼리는 메로리 공유가 불가능
3.1개의 그룹에 속하는 Thread 수는 [numthread(x,y,z)]로 정의하고 그룹에는 [x*y*z]개의
  Thread가 존재한다.
4.한 그룹당 최대 Thread수는 768개
5.CS에서 여러개의 GroupID(그룹)을 구동 시킬 수 있으며, 각각의 그룹은 여러개의 Thread로
  구성되어 있다. 각각의 그룹과 Thread는 3차원 좌표의 위치로 식별된다.
6.자신이 어떤 Thread그룹의 어떤 Thread인가를 이들 시스템값(dispatchID)을 사용하여 알 수 있다.

GroupID -> 계산 쉐이더가 실행중인 스레드 그룹에 대한 인덱스입니다.
GroupThreadID -> 계산 쉐이더가 실행중인 스레드 그룹 내의 개별 스레드에 대한 인덱스입니다. 
DispatchThreadID -> 계산 쉐이더가 실행중인 스레드와 스레드 그룹을 결합한 인덱스입니다. 

DispatchTreadID = (GroupID * numthreads) + GroupThreadID

ex)for문 0부터 8까지 돌리고 Dispatch (2,1,1), numthreads (4,1,1)이 호출되면

0(GroupID)[0(DispatchTreadID)][1][2][3]
           0(GroupThreadID)    1  2  3 
1[4][5][6][7]
  0  1  2  3

ex)위와 같이 숫자 8개 Dispatch (1,1,1)이면 numthreads (8,1,1)이 된다

0[0][1][2][3][4][5][6][7]
  0  1  2  3  4  5  6  7
*/
