
#if !defined(_THREAD_HPP)
#define _THREAD_HPP

#include <process.h>


// void * _threadstore()	fÅr den aktuellen thread

class VThread
{
public:
	VThread()	{ ended = 0; threadId = -1; }
	~VThread()	{}

	int	run()
		    {
		    threadId = _beginthread(threadStartHelper, 0, 8192, this);
		    return (threadId != -1 ? 0 : 1);
		    }

	int	queryEnded()	{ return ended; }
	int	queryResult()	{ return result; }

	virtual int threadWorker() = 0;
private:
	int	threadId;
	int	ended;
	int	result;

protected:
	friend void _Optlink threadStartHelper(void *object);
};



class VDosThread : public VThread
{
public:
	VDosThread()	{}
	~VDosThread()	{}
};

#endif /*_THREAD_HPP*/
