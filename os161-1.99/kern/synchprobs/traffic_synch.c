#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>
#include <opt-A1.h>
void enterorleave(Direction origin, Direction destination,int type);
bool checkrightturn(Direction origin, Direction destination);
bool checkconflict(Direction origin, Direction destination);
/* 
 * This simple default synchronization mechanism allows only vehicle at a time
 * into the intersection.   The intersectionSem is used as a a lock.
 * We use a semaphore rather than a lock so that this code will work even
 * before locks are implemented.
 */

/* 
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to 
 * declare other global variables if your solution requires them.
 */

/*
 * replace this with declarations of any synchronization and other variables you need here
 */
 //static struct semaphore *intersectionSem;
static struct cv *conflict_cv, *noconflict_cv;
static struct lock *mutex;
bool checkrightturn(Direction origin, Direction destination)//check if curent car is making right turn
{
 // kprintf("checkcalled ");
  //kprintf("");
  int dir,dest;
  switch (origin)
  {
    case north:
    dir=0;
    break;
    case east:
    dir=1;
    break;
    case south:
    dir=2;
    break;
    case west:
    dir=3;
    break;
  }
  switch (destination)
  {
    case north:
    dest=0;
    break;
    case east:
    dest=1;
    break;
    case south:
    dest=2;
    break;
    case west:
    dest=3;
    break;
  }
  //kprintf("  dir is %d dest is %d",dir,dest);

  if(dir==0 &&  dest==3)
  {
    return true;
  }
  else if(dir==2 &&  dest==1)
  {
    return true;
  }
  else if(dir==3 &&  dest==2)
  {
    return true;
  }
  else if(dir==1 &&  dest==0)
  {
    return true;
  }
  else
  {
    //  kprintf("not right turn!!!\n");

    return false;
  }
}
int trafic [4][4]={{0}};// trafic array is used to store the cars in the 
bool checkconflict(Direction origin, Direction destination)
{
  int dir=0;
  int dest=0;
  switch (origin)
  {
    case north:
    dir=0;
    break;
    case east:
    dir=1;
    break;
    case south:
    dir=2;
    break;
    case west:
    dir=3;
    break;
  }
  switch (destination)
  {
    case north:
    dest=0;
    break;
    case east:
    dest=1;
    break;
    case south:
    dest=2;
    break;
    case west:
    dest=3;
    break;
  }
   // kprintf("  check called dir is %d dest is %d",dir,dest);

  int temptrafic [4][4];
  int i =0;
  int j = 0;
  for(i=0;i<4;i++)
  {
    for(j=0;j<4;j++)
    {
        temptrafic[i][j]=trafic[i][j];//copy the current trafic condition
      }
    }
    for(j=0;j<4;j++)
    {
      temptrafic[dir][j]=0;//remove all car from same direction
    }
    temptrafic[dest][dir]=0;//remove oppsite
    for(i=0;i<4;i++)
    {
      for(j=0;j<4;j++)
      {
        if(j!=dest)
        {
          if(checkrightturn(origin,destination))
          {
            temptrafic[i][j]=0;
          }
          else
          {
            switch (i)
            {
              case 0:
              if(dest!=3){
              temptrafic[0][3]=0;}
              break;
              case 1:
              if(dest!=0){
              temptrafic[1][0]=0;}
              break;
              case 2:
              if(dest!=1){
              temptrafic[2][1]=0;}
              break;
              case 3:
              if(dest!=2){
              temptrafic[3][2]=0;}
              break;
            }
          }
        }
      }
    }
    for(i=0;i<4;i++)
    {
      for(j=0;j<4;j++)
      {
        if(temptrafic[i][j]!=0)
        {
            //kprintf("conflict \n");

          return true;

        }
      }
    }
              //  kprintf("no conflict \n");

    return false;
  }
void enterorleave(Direction origin, Direction destination,int type)
{
  int dir,dest;
  switch (origin)
  {
    case north:
    dir=0;
    break;
    case east:
    dir=1;
    break;
    case south:
    dir=2;
    break;
    case west:
    dir=3;
    break;
  }
  switch (destination)
  {
    case north:
    dest=0;
    break;
    case east:
    dest=1;
    break;
    case south:
    dest=2;
    break;
    case west:
    dest=3;
    break;
  }
    //kprintf("  enter called dir is %d dest is %d",dir,dest);

  if(type==1){
  trafic[dir][dest]++;
    //  kprintf("  enter ");

  }
  else
  {
    trafic[dir][dest]--;
          //kprintf("  leave ");

  }

  return;
}
/* 
 * The simulation driver will call this function once before starting
 * the simulation
 *
 * You can use it to initialize synchronization and other variables.
 * 
 */
 void
 intersection_sync_init(void)
 {
  /* replace this default implementation with your own implementation */

  //intersectionSem = sem_create("intersectionSem",1);
  //if (intersectionSem == NULL) {
    //panic("could not create intersection semaphore");
  //}
  conflict_cv=cv_create("conflict_cv");
  if (conflict_cv == NULL) {
    panic("could not create conflict_cv");}
  noconflict_cv=cv_create("noconflict_cv");
  if (noconflict_cv == NULL) {
    panic("could not create noconflict_cv");}
  mutex=lock_create("mutex");
  if (mutex == NULL) {
    panic("could not create mutex");
  }
  return;
}

/* 
 * The simulation driver will call this function once after
 * the simulation has finished
 *
 * You can use it to clean up any synchronization and other variables.
 *
 */
 void
 intersection_sync_cleanup(void)
 {
  /* replace this default implementation with your own implementation */
 // KASSERT(intersectionSem != NULL);
  //sem_destroy(intersectionSem);
  KASSERT(conflict_cv != NULL);
  cv_destroy(conflict_cv);
  KASSERT(noconflict_cv != NULL);
  cv_destroy(noconflict_cv);
  KASSERT(mutex != NULL);
  lock_destroy(mutex);
}


/*
 * The simulation driver will call this function each time a vehicle
 * tries to enter the intersection, before it enters.
 * This function should cause the calling simulation thread 
 * to block until it is OK for the vehicle to enter the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle is arriving
 *    * destination: the Direction in which the vehicle is trying to go
 *
 * return value: none
 */

 void
 intersection_before_entry(Direction origin, Direction destination) 
 {
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
 // KASSERT(intersectionSem != NULL);
 // P(intersectionSem);
  lock_acquire(mutex);
  while(checkconflict(origin,destination))
  {
    cv_wait(conflict_cv,mutex);
  }
  enterorleave(origin,destination,1);
 // cv_signal(noconflict_cv,mutex);
  lock_release(mutex);
}


/*
 * The simulation driver will call this function each time a vehicle
 * leaves the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle arrived
 *    * destination: the Direction in which the vehicle is going
 *
 * return value: none
 */

 void
 intersection_after_exit(Direction origin, Direction destination) 
 {
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  //KASSERT(intersectionSem != NULL);
  //V(intersectionSem);
  lock_acquire(mutex);
  enterorleave(origin,destination,0);
  cv_broadcast(conflict_cv,mutex);
  lock_release(mutex);
}


