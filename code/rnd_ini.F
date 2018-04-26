      SUBROUTINE RND_INI(ISEQ)
C---       RANLUX initialization (random number)
      IMPLICIT NONE
      INTEGER ISEQ
      INTEGER k1,k2,lux
C
!  Jixie: If given seed is zero, create a seed using PID and system time
      integer*4 num,idum,pid,GetPId   
      integer*8 unixtime
      character*30  cdate

      if(ISEQ.EQ.0) THEN
         unixtime = time8()
         call ctime(unixtime,cdate)  

         pid=GetPId() 
         num=10*pid
         idum=int(float(num)*abs(
     &        float(unixtime-123736761)/float(unixtime+123736761)
     &       -float(num-697899)/float(num+697899)
     &       +float(pid-3835)/float(pid+3835)))

         ISEQ=idum
         write(6,*) 'pid, unixtime, seed=',pid,unixtime,idum
         write(6,*) 'Create seed(',idum,') from PID and start time: '
     +   ,cdate
      ENDIF
      
      k1=0
      k2=0
      lux=3
      CALL RLUXGO(lux,ISEQ,k1,k2)
C
      RETURN
      END
