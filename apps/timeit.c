#include <sys/time.h>
#include <stdio.h>
#include <windows.h>

BOOL called_back;

#define NANOSECONDS_PER_SECOND 1000000000

struct timespec diff_time( struct timespec before, struct timespec after){
    struct timespec result = {0};

    if ((after.tv_sec < before.tv_sec) || ((after.tv_sec == before.tv_sec) && (after.tv_nsec <= before.tv_nsec))) { /* after before before */
        result.tv_sec = result.tv_nsec = 0;
    }
    result.tv_sec = after.tv_sec - before.tv_sec;
    result.tv_nsec= after.tv_nsec- before.tv_nsec;
    if (result.tv_nsec<0) {
        result.tv_sec--;
        /* result.tv_nsec is negative, therefore we use "+" */
        result.tv_nsec = NANOSECONDS_PER_SECOND + result.tv_nsec;
    }
    return result;
}

VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired){
    clock_gettime(CLOCK_MONOTONIC, lpParameter);
    called_back = TRUE;
}

int main(int argc, char *argv[]){

    if(argc == 1){
        printf("No arguments provided\n");
        return 1;
    }

    struct timespec before = {0};
    struct timespec after = {0};
    struct timespec diff = {0};

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    HANDLE hwait;

    called_back = FALSE;

    for(int i = 1;i < argc;i++){
        if(CreateProcessA((const char *) argv[i], NULL, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi) == FALSE){
            printf("%lx\n", GetLastError());
            goto CLEANUP;
        }

        clock_gettime(CLOCK_MONOTONIC, &before);


        if(RegisterWaitForSingleObject(&hwait, pi.hProcess, WaitOrTimerCallback, &after, INFINITE, WT_EXECUTEONLYONCE) == FALSE){
            printf("%lx\n", GetLastError());
            goto CLEANUP;
        }
            
        while(called_back == FALSE){
            Sleep(10);
        }

        diff = diff_time(before, after);
        printf("S: %ld.%.3ld\n", diff.tv_sec, diff.tv_nsec / 1000000);

        CLEANUP:
        UnregisterWait(hwait);
        CloseHandle(si.hStdOutput);
        CloseHandle(si.hStdError);
        CloseHandle(si.hStdInput);
        CloseHandle(pi.hProcess);
    }

    return 0;
}
