
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

int readySize;
int runningSize;
int waitingSize;
int exitSize;

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
void RR();
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
  ManagementInitialization();
  while (1) {
    IO();
    CPUScheduler();
    Dispatcher();
  }
}

void IO() {
  int i;
  ProcessControlBlock *PCB = Queues[RUNNINGQUEUE].Tail; // Points to tail of RUNNINGQUEUE
  for(i = 0; i < 1; i++) { // If RUNNINGQUEUE is empty, still need to check WAITINGQUEUE
    if (PCB == NULL) { // If no PCB in RUNNINGQUEUE, Return
      break;
    }
    PCB->TimeEnterWaiting = Now(); // Set TimeEnterWaiting
    EnqueueProcess(WAITINGQUEUE, DequeueProcess(RUNNINGQUEUE)); // Place PCB in WAITINGQUEUE
    PCB->TimeIOBurstDone = Now() + PCB->IOBurstTime; // Set time IO burst will be completed
    runningSize--;
    waitingSize++;
    PCB->state = WAITING; // Update PCB state
  }

  for(i = 0; i < waitingSize; i++) { // Scan the WAITINGQUEUE
    ProcessControlBlock *PCB = Queues[WAITINGQUEUE].Tail; // Points to tail of WAITINGQUEUE
    if(PCB->TimeIOBurstDone <= Now()) { // If PCB IO burst is done
      if(PCB->TotalJobDuration <= PCB->TimeInCpu) { // If PCB job is done
        EnqueueProcess(EXITQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in EXITQUEUE from WAITINGQUEUE
        PCB->JobExitTime = Now(); // Set JobExitTime
        waitingSize--;
        exitSize++;
        PCB->state = DONE; // Update PCB state
      }
      else { // PCB job is not done
        PCB->TimeInWaitQueue += Now() - PCB->TimeEnterWaiting; // Update TimeInWaitQueue
        PCB->JobStartTime = Now(); // Update JobStartTime
        EnqueueProcess(READYQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in READYQUEUE from WAITINGQUEUE
        waitingSize--;
        readySize++;
        PCB->state = READY; // Update PCB state
      }
      continue;
    }
    EnqueueProcess(WAITINGQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in front of WAITINGQUEUE from back of WAITINGQUEUE
  }
}

void CPUScheduler() {
  switch (PolicyNumber) {
    case 1:
    FCFS();
    break;

    case 2:
    SRTF();
    break;

    case 3:
    RR();
    break;

    default:
    break;
  }
}

void FCFS() {
  ProcessControlBlock *PCB = Queues[READYQUEUE].Tail; // Points to tail of READYQUEUE
  if (runningSize >= 1 || PCB == NULL) { // If no PCB in READYQUEUE or RUNNINGQUEUE is full, Return
    return;
  }
  EnqueueProcess(RUNNINGQUEUE, DequeueProcess(READYQUEUE)); // Place PCB in RUNNINGQUEUE from READYQUEUE
  readySize--;
  runningSize++;
  PCB->state = RUNNING; // Update PCB state
}

void SRTF() {
  int i;
  ProcessControlBlock *PCB1 = Queues[READYQUEUE].Tail; // Points to tail of READYQUEUE
  ProcessControlBlock *PCB2;
  ProcessControlBlock *PCB3;
  if (runningSize >= 1 || PCB1 == NULL) { // If no PCB in READYQUEUE or RUNNINGQUEUE is full, Return
    return;
  }

  for(i = 0; i < readySize; i++) { // Scan the runningQueue
    PCB2 = Queues[READYQUEUE].Tail;
    if (PCB2->TotalJobDuration - PCB2->TimeInCpu < PCB1->TotalJobDuration - PCB1->TimeInCpu) { // If PCB2 SRT < PCB1 SRT, PCB1 = PCB2
      PCB1 = PCB2;
    }
    EnqueueProcess(READYQUEUE, DequeueProcess(READYQUEUE)); // Place PCB in front of READYQUEUE from back of READYQUEUE
  }

  for(i = 0; i < readySize; i++) { // Scan the RUNNINGQUEUE
    PCB3 = Queues[READYQUEUE].Tail;
    if (PCB1 == PCB3) { // If currently at the minimum SRT, remove it from READYQUEUE and move to RUNNINGQUEUE
      EnqueueProcess(RUNNINGQUEUE, DequeueProcess(READYQUEUE));
      readySize--;
      runningSize++;
      PCB1->state = RUNNING;
      return;
    }
    EnqueueProcess(READYQUEUE, DequeueProcess(READYQUEUE));  // Place PCB in front of READYQUEUE from back of READYQUEUE
  }
}

void RR() {
  ProcessControlBlock *PCB = Queues[READYQUEUE].Tail; // Points to tail of READYQUEUE
  if (runningSize >= 1 || PCB == NULL) { // If no PCB in READYQUEUE or RUNNINGQUEUE is full, Return
    return;
  }
  if(PCB->CpuBurstTime > Quantum) { // If PCB CpuBurstTime > quantum, CpuBurstTime = quantum
    PCB->CpuBurstTime = Quantum;
  }
  EnqueueProcess(RUNNINGQUEUE, DequeueProcess(READYQUEUE)); // Place PCB in RUNNINGQUEUE from READYQUEUE
  readySize--;
  runningSize++;
  PCB->state = RUNNING; // Update PCB state
}

void Dispatcher() {
  double start;
  ProcessControlBlock *PCB = Queues[RUNNINGQUEUE].Tail; // Points to tail of RUNNINGQUEUE
  if (PCB == NULL) { // If no PCB in RUNNINGQUEUE, Return
    return;
  }
  PCB->TimeInReadyQueue += Now() - PCB->JobStartTime; // Set TimeInReadyQueue
  if(PCB->TimeInCpu == 0) { // If first time in CPU
    PCB->StartCpuTime = Now(); // Set StartCpuTime
  }
  OnCPU(PCB, PCB->CpuBurstTime); // Simulate CPU usage
  PCB->TimeInCpu += PCB->CpuBurstTime; // Set TimeInCpu
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
  NewProcess->TimeInCpu = 0; // Fixes TUX error
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}

void BookKeeping(void){
  double end = Now(); // Total time for all processes to arrive
  int i;
  double ATAT;
  double ART;
  double CBT;
  double T;
  double AWT;
  ProcessControlBlock *PCB;

  for(i = 0; i < readySize; i++) { // Scan the READYQUEUE, sum all data
    PCB = Queues[READYQUEUE].Tail; // Points to tail of READYQUEUE
    CBT += PCB->TimeInCpu;
    AWT += PCB->TimeInReadyQueue;
    // if(PCB->StartCpuTime != 0) { // Does not work on TUX, but logically correct
    //   //ART += PCB->StartCpuTime - PCB->JobArrivalTime;
    // }
    EnqueueProcess(READYQUEUE, DequeueProcess(READYQUEUE)); // Place PCB in READYQUEUE
  }

  for(i = 0; i < runningSize; i++) { // Scan the RUNNINGQUEUE, sum all data
    PCB = Queues[RUNNINGQUEUE].Tail; // Points to tail of RUNNINGQUEUE
    CBT += PCB->TimeInCpu;
    AWT += PCB->TimeInReadyQueue;
    if(PCB->StartCpuTime != 0) { // Does not work on TUX, but logically correct
      ART += PCB->StartCpuTime - PCB->JobArrivalTime;
    }
    EnqueueProcess(RUNNINGQUEUE, DequeueProcess(RUNNINGQUEUE)); // Place PCB in RUNNINGQUEUE
  }

  for(i = 0; i < waitingSize; i++) { // Scan the WAITINGQUEUE, sum all data
    PCB = Queues[WAITINGQUEUE].Tail; // Points to tail of WAITINGQUEUE
    CBT += PCB->TimeInCpu;
    AWT += PCB->TimeInReadyQueue;
    ART += PCB->StartCpuTime - PCB->JobArrivalTime;
    EnqueueProcess(WAITINGQUEUE, DequeueProcess(WAITINGQUEUE)); // Place PCB in WAITINGQUEUE
  }

  for(i = 0; i < exitSize; i++) { // Scan the EXITQUEUE, sum all data
    PCB = Queues[EXITQUEUE].Tail; // Points to tail of EXITQUEUE
    ATAT += PCB->JobExitTime - PCB->JobArrivalTime;
    CBT += PCB->TimeInCpu;
    AWT += PCB->TimeInReadyQueue;
    ART += PCB->StartCpuTime - PCB->JobArrivalTime;
    EnqueueProcess(EXITQUEUE, DequeueProcess(EXITQUEUE)); // Place PCB in EXITQUEUE
  }

  // Compute averages and final results
  ATAT /= exitSize;
  ART /= exitSize;
  CBT /= end;
  T = exitSize / end;
  AWT /= exitSize;

  printf("\n********* My Numbers *************************************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", exitSize);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f\n", ATAT, ART, CBT, T, AWT);

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
  ProcessControlBlock *PCB = Queues[JOBQUEUE].Tail; // Points to tail of JOBQUEUE
  if (PCB == NULL) { // If no PCB in JOBQUEUE, Return
    return;
  }
  PCB->TimeInJobQueue = Now() - PCB->JobArrivalTime; // Set TimeInJobQueue
  PCB->JobStartTime = Now(); // Set JobStartTime
  EnqueueProcess(READYQUEUE, DequeueProcess(JOBQUEUE)); // Place PCB in READYQUEUE from JOBQUEUE
  readySize++;
  PCB->state = READY; // Update PCB state
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
  readySize = 0;
  runningSize = 0;
  waitingSize = 0;
  exitSize = 0;
  return TRUE;
}
