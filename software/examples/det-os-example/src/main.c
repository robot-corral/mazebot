#include "det_os.h"
#include "det_os_implementation.h"

void startTask(void* pTaskParameter)
{
}

void main()
{
    runDetOs(startTask);
}
