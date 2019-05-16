#include "Thread.h"

Thread::Thread()
:m_stopFlag(false)
, m_hThread(INVALID_HANDLE_VALUE)
{
}

Thread::~Thread()
{
	Stop();
}

void Thread::Start()
{
	unsigned long *p = NULL;
	m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, p);
}

DWORD WINAPI Thread::ThreadProc(LPVOID p)
{
	Thread* thread = (Thread*)p;
	thread->Run();

	CloseHandle(thread->m_hThread);
	thread->m_hThread = INVALID_HANDLE_VALUE;

	return 0;
}

void Thread::Stop()
{
	m_stopFlag = true;

	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		if (WaitForSingleObject(m_hThread, INFINITE) != WAIT_ABANDONED)
		{
			CloseHandle(m_hThread);
		}
		m_hThread = INVALID_HANDLE_VALUE;
	}
}


bool Thread::IsStop()
{
	return m_stopFlag;
}