
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 5/9/2017 for COMP 3500 labs                                         *
* Date  : February 20, 2009                                                   *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/



/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 10 /* XX */




/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

char choice;
int quantum;
int readySize = 0;
int runningSize = 0;
int waitingSize = 0;
int exitSize = 0;
double lastReady;
int q = 0;

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void                 ManageProcesses(void);
void                 NewJobIn(ProcessControlBlock whichProcess);
void                 BookKeeping(void);
Flag                 ManagementInitialization(void);
void                 LongtermScheduler(void);
void IO();
void CPUScheduler();
void FCFS();
void SRTF();
void RR(int quantum);
void Dispatcher();

/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment operating systems. The parent    *
*           process is the "Operating Systems" managing the processes using   *
*           the resources (CPU and Memory) of the system                      *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
  if (argc == 2) { // One arguments
    choice = *argv[1]; // Algorithm choice
  }
  if (argc == 3 || argc == 4) { // Two or three arguments
    choice = *argv[1]; // Algorithm choice
    quantum = *argv[2] - '0'; // Quantum
  }
  if (Initialization(argc,argv)){
    ManageProcesses();
  }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Sources and process events (written by students)    *
\***********************************************************************/

void ManageProcesses(void){
  ManagementInitialization(); // Initialize Resources??
  while (1) {
    IO();
    CPUScheduler();
    Dispatcher();
  }
}

void IO() {
  ProcessControlBlock *PCB = Queues[2].Tail; // Points to tail of RUNNINGQUEUE
  for(int i = 0; i < 1; i++) { // If RUNNINGQUEUE is empty, still need to check WAITINGQUEUE
    if (PCB == NULL) { // If no PCB in RUNNINGQUEUE, Return
      break;
    }
    PCB->TimeEnterWaiting = Now(); // Set TimeEnterWaiting
    EnqueueProcess(WAITINGQUEUE, DequeueProcess(RUNNINGQUEUE)); // Place PCB in WAITINGQUEUE
    runningSize--;
    waitingSize++;
    PCB->state = WAITING;
    PCB->TimeIOBurstDone = Now() + PCB->IOBurstTime; // Set time IO burst will be completed

    printf("%f", Now());
    printf(" *** PID: ");
    printf("%d", PCB->ProcessID);
    printf(" placed in IO. Time IO Done: ");
    printf("%f", PCB->TimeIOBurstDone);
    printf("\n");
  }
  for(int i = 0; i < waitingSize; i++) { // Scan the WAITINGQUEUE
    ProcessControlBlock *PCB = Queues[3].Tail; // Points to tail of WAITINGQUEUE
    if(PCB->TimeIOBurstDone <= Now()) { // If PCB IO burst is done

      printf("%f", Now());
      printf(" IO Done for PID: ");
      printf("%d", PCB->ProcessID);
      printf("\n");

      if(PCB->TotalJobDuration <= PCB->TimeInCpu) { // If PCB job is done
        PCB->JobExitTime = Now(); // Set JobExitTime
        EnqueueProcess(EXITQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in EXITQUEUE from WAITINGQUEUE
        waitingSize--;
        exitSize++;
        PCB->state = DONE; // Update PCB state

        printf("%f", Now());
        printf(" *** PID ");
        printf("%d", PCB->ProcessID);
        printf(" job completed. Placed in EXITQUEUE.\n");

        q++;
        if(q >= 10) {
          BookKeeping();
        }
      }
      else { // PCB job is not done
        PCB->TimeInWaitQueue = PCB->TimeInWaitQueue + Now() - PCB->TimeEnterWaiting; // Update TimeInWaitQueue
        lastReady = Now(); // Used to calculate TimeInReadyQueue
        EnqueueProcess(READYQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in READYQUEUE from WAITINGQUEUE
        waitingSize--;
        readySize++;
        PCB->state = READY; // Update PCB state

        printf("%f", Now());
        printf(" *** PID ");
        printf("%d", PCB->ProcessID);
        printf(" job not completed. Placed in READYQUEUE.\n");
      }
      continue;
    }
    EnqueueProcess(WAITINGQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in front of WAITINGQUEUE from back of WAITINGQUEUE
  }
}

void CPUScheduler() {
  switch (choice) {
    case '1':
    FCFS();
    break;

    case '2':
    SRTF();
    break;

    case '3':
    RR(quantum);
    break;

    default:
    break;
  }
}

void FCFS() {
  ProcessControlBlock *PCB = Queues[1].Tail; // Points to tail of READYQUEUE
  if (runningSize >= 1 || PCB == NULL) { // If no PCB in READYQUEUE, Return
    return;
  }
  EnqueueProcess(RUNNINGQUEUE, DequeueProcess(READYQUEUE)); // Place PCB in RUNNINGQUEUE from READYQUEUE
  readySize--;
  runningSize++;
  PCB->state = RUNNING; // Update PCB state

  printf("%f", Now());
  printf(" *** PID ");
  printf("%d", PCB->ProcessID);
  printf(" in RUNNINGQUEUE.\n");
}

void SRTF() {
  ProcessControlBlock *PCB1 = Queues[1].Tail; // Points to tail of READYQUEUE
  if (runningSize >= 1 || PCB1 == NULL) { // If no PCB in READYQUEUE, Return
    return;
  }
  ProcessControlBlock *PCB2 = Queues[1].Tail;
  for(int i = 0; i < runningSize; i++) { // Scan the runningQueue
    if (PCB2->TotalJobDuration - PCB2->TimeInCpu < PCB1->TotalJobDuration - PCB1->TimeInCpu) {
      PCB1 = PCB2;
    }
    EnqueueProcess(READYQUEUE, DequeueProcess(READYQUEUE));
  }
  EnqueueProcess(RUNNINGQUEUE, PCB1);
  readySize--;
  runningSize++;
  PCB1->state = RUNNING;

  printf("%f", Now());
  printf(" *** PID ");
  printf("%d", PCB1->ProcessID);
  printf(" in RUNNINGQUEUE.\n");
}

void RR(int quantum) {

}

void Dispatcher() {
  ProcessControlBlock *PCB = Queues[2].Tail; // Points to tail of RUNNINGQUEUE
  if (PCB == NULL) { // If no PCB in RUNNINGQUEUE, Return
    return;
  }

  printf("%f", Now());
  printf(" *** PID ");
  printf("%d", PCB->ProcessID);
  printf(" selected for CPU.\n");

  if(PCB->TimeInReadyQueue == 0) {
    PCB->TimeInReadyQueue = Now() - PCB->JobStartTime; // Set TimeInReadyQueue

    // printf("&&& TimeInReadyQueue: ");
    // printf("%f", PCB->TimeInReadyQueue);
    // printf("\n");
  }
  else {
    PCB->TimeInReadyQueue = PCB->TimeInReadyQueue + Now() - lastReady; // Update TimeInReadyQueue

    // printf("&&& TimeInReadyQueue: ");
    // printf("%f", PCB->TimeInReadyQueue);
    // printf("\n");
  }
  if(PCB->TimeInCpu == 0) { // If first time in CPU
    PCB->StartCpuTime = Now(); // Set StartCpuTime

    printf("---------------------\nTotalJobDuration: ");
    printf("%f", PCB->TotalJobDuration);
    printf("\n");
    printf("CpuBurstTime: ");
    printf("%f", PCB->CpuBurstTime);
    printf("\n");
    printf("IOBurstTime: ");
    printf("%f", PCB->IOBurstTime);
    printf("\n---------------------\n");
  }
  OnCPU(PCB, PCB->CpuBurstTime); // Simulate CPU usage

  PCB->TimeInCpu = PCB->TimeInCpu + PCB->CpuBurstTime; // Set TimeInCpu

  printf("%f", Now());
  printf(" *** PID ");
  printf("%d", PCB->ProcessID);
  printf(" done with CPU. CPU time: ");
  printf("%f", PCB->TimeInCpu);
  printf("\n");
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run when a job is added to the Job Queue    *
\***********************************************************************/

void NewJobIn(ProcessControlBlock whichProcess){
  ProcessControlBlock *NewProcess;

  /* Add Job to the Job Queue */
  NewProcess = (ProcessControlBlock *) malloc(sizeof(ProcessControlBlock));
  memcpy(NewProcess,&whichProcess,sizeof(whichProcess));
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}

void BookKeeping(void){
  double end = Now();
  double ATAT;
  double ART;
  double CBT;
  double T;
  double AWT;
  ProcessControlBlock *PCB;

  DisplayQueue("Ready Queue", READYQUEUE);
  DisplayQueue("Waiting Queue", WAITINGQUEUE);
  DisplayQueue("Running Queue", RUNNINGQUEUE);
  DisplayQueue("Exit Queue", EXITQUEUE);
  printf("*****************\nReady Size: %d Waiting Size: %d Running Size: %d Exit Size %d\n", readySize, waitingSize, runningSize, exitSize);
  printf("Number of Completed Processes = ");
  printf("%d", exitSize);
  printf("\n");

  for(int i = 0; i < exitSize; i++) { // Scan the EXITQUEUE
    ProcessControlBlock *PCB = Queues[4].Tail; // Points to tail of EXITQUEUE
    //printf("-----------------\nPID: ");
    //printf("%d", PCB->ProcessID);
    //printf("\n");
    //printf("JobExitTime: ");
    //printf("%f", PCB->JobExitTime);
    //printf("\n");
    //printf("JobArrivalTime: ");
    //printf("%f", PCB->JobArrivalTime);
    //printf("\n");
    //printf("StartCpuTime: ");
    //printf("%f", PCB->StartCpuTime);
    //printf("\n");
    //printf("TimeInCpu: ");
    //printf("%f", PCB->TimeInCpu);
    //printf("\n");
    //printf("TimeInReadyQueue: ");
    //printf("%f", PCB->TimeInReadyQueue);
    //printf("\n-----------------\n");

    ATAT = ATAT + PCB->JobExitTime - PCB->JobArrivalTime;

    //printf("%f", ATAT);
    //printf("\n");

    //double RT = PCB->StartCpuTime - PCB->JobArrivalTime;
    //printf("StartCpuTime: ");
    //printf("%f", PCB->StartCpuTime);
    //printf("\n");
    //printf("JobArrivalTime: ");
    //printf("%f", PCB->JobArrivalTime);
    //printf("\n");
    //printf("Response time: ");
    //printf("%f", RT);
    //printf("\n");

    ART = ART + PCB->StartCpuTime - PCB->JobArrivalTime;
    CBT = CBT + PCB->TimeInCpu;
    AWT = AWT + PCB->TimeInReadyQueue;

    //double WT = PCB->TimeInReadyQueue;
    //printf("Waiting Time: ");
    //printf("%f", WT);
    //printf("\n-----------------\n");
    //printf("JobArrivalTime: ");
    //printf("%f", PCB->JobArrivalTime);
    //printf("\n");

    EnqueueProcess(EXITQUEUE, DequeueProcess(EXITQUEUE)); // Place PCB in EXITQUEUE
  }
  ATAT = ATAT / exitSize;
  ART = ART / exitSize;
  CBT = CBT / end;
  T = exitSize / end;
  AWT = AWT / exitSize;

  printf("ATAT: ");
  printf("%f", ATAT);
  printf("\n");
  printf("ART: ");
  printf("%f", ART);
  printf("\n");
  printf("CBT: ");
  printf("%f", CBT);
  printf("\n");
  printf("T: ");
  printf("%f", T);
  printf("\n");
  printf("AWT: ");
  printf("%f", AWT);
  printf("\n");

  exit(0);
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: Decides which processes should be admitted in Ready Queue   *
*           If enough memory and within multiprogramming limit,         *
*           then move Process from Job Queue to Ready Queue             *
\***********************************************************************/

void LongtermScheduler(void){
  ProcessControlBlock *PCB = Queues[0].Tail; // Points to tail of JOBQUEUE
  if (PCB == NULL) { // If no PCB in JOBQUEUE, Return
    return;
  }
  PCB->TimeInJobQueue = Now() - PCB->JobArrivalTime; // Set TimeInJobQueue
  PCB->JobStartTime = Now(); // Set JobStartTime
  EnqueueProcess(READYQUEUE, DequeueProcess(JOBQUEUE)); // Place PCB in READYQUEUE
  readySize++;
  PCB->state = READY; // Update PCB state

  printf("%f", Now());
  printf(" *** PID ");
  printf("%d", PCB->ProcessID);
  printf(" in READYQUEUE.\n");
}

/***********************************************************************\
* Input : Priority level                                                *
* Output: Pointer to Process dequeued, (NULL if queue empty)            *
* Function: Denqueues an event, returns pointer to that event           *
\***********************************************************************/


/***********************************************************************\
* Input : Priority level where to start searching for free spot         *
* Output: Priority Level of queue with free spot, Zero if no free spot  *
\***********************************************************************/



/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Intialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  return TRUE;
}
