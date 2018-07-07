

#include "thread.hpp"



void _Optlink threadStartHelper(void *object)
{
    ((VThread *)object)->result = ((VThread *)object)->threadWorker();
    ((VThread *)object)->ended = 1;
    _endthread();
}

