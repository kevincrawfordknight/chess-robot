
#=======================================================================
# Puts the robot "gaze" on the red spot in the environment.   If the red 
# spot is in a known position, for example, this serves to orient the 
# robot into a known position (before it executes some recipe movement).
#
# Usage:
#
#  % fs.correct.y xx 37 1 red 40
#
# Positions the red dot at position <50,37> in the 100x75 camera image
# by moving robot arm #1.  (#1 and #2 are used when two robot arms are
# plugged into the same Pi).  Fuzz factor = 40.
#
# Method (for each axis): 
#   Find coord of dot in camera image.
#   Rotate X seconds toward the dot.
#   Find coord of dot in camera image again.
#   Estimate how many more seconds to rotate (& direction) to reach dot.
#   Rotate again toward the dot.
#   Report final coord of dot in camera image.  
#=======================================================================

ARM=$3
XGOAL=$1     # goal coordinates for red spot in image/gaze
YGOAL=$2     
XPPS=23      # very rough pixels per second in x direction (via shoulder rotation)
YPPS=30      # very rough pixels per vertical second in y direction (via elbow)
ALPHA=0.35   # compensate for gravity pulling elbow down
FUZZ=$5
COLOR=$4

#==================================================
# determine y-coord of red spot
#==================================================

/home/pi/work/lincoln/camera/takepic board.jpg
display board.jpg&
convert board.jpg -fuzz $FUZZ\% -fill cyan +opaque $COLOR -colorspace GRAY -posterize 2 -fuzz 15% -fill $COLOR -opaque black board3.jpg

Y1=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | awk '{print $2}' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`

echo '*** y1='$Y1
display board3.jpg&

#==================================================
# rotate elbow in direction of red spot
#==================================================

YDONE=0

if (( (($Y1<=$YGOAL)&&(($YGOAL-$Y1)<=2))||(($YGOAL<=$Y1)&&(($Y1-$YGOAL)<=2)) ))
  then move$ARM 00 00 00 0.1
       YDONE=1
       Y2=Y1
  elif [ "$Y1" -lt "$YGOAL" ] 
       then move$ARM 10 00 00 `echo "($YGOAL-$Y1)/$YPPS" | bc -l`
            SEC=`echo "($YGOAL-$Y1)/$YPPS" | bc -l`
  else move$ARM 20 00 00 `echo "(($Y1-$YGOAL)/$YPPS)*$ALPHA" | bc -l`
       SEC=`echo "(($Y1-$YGOAL)/$YPPS)*$ALPHA" | bc -l`
fi

#==================================================
# take another picture
#==================================================

if (( ($YDONE==0) ))
  then
    /home/pi/work/lincoln/camera/takepic board.jpg
    display board.jpg&

#==================================================
# determine new y-coord of red spot
#==================================================

    convert board.jpg -fuzz $FUZZ\% -fill cyan +opaque $COLOR -colorspace GRAY -posterize 2 -fuzz 15% -fill $COLOR -opaque black board3.jpg
    Y2=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | awk '{print $2}' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`
    echo '*** y2='$Y2
    display board3.jpg&
fi

#==================================================
# figure out how much to move elbow & move it
#==================================================

if (( (($YDONE==1)||(($Y2<=$YGOAL)&&(($YGOAL-$Y2)<=2))||(($YGOAL<=$Y2)&&(($Y2-$YGOAL)<=2))) ))
  then move$ARM 00 00 00 0.1
       YDONE=1
  elif [ "$Y1" -gt "$Y2" ] && [ "$Y2" -gt "$YGOAL" ]
    then move$ARM 20 00 00 `echo "($Y2-$YGOAL)/($Y1-$Y2)*$ALPHA*$SEC" | bc -l`
  elif [ "$Y1" -gt "$YGOAL" ] && [ "$YGOAL" -gt "$Y2" ]
    then move$ARM 10 00 00 `echo "($YGOAL-$Y2)/($Y1-$Y2)*$SEC" | bc -l`
  elif [ "$YGOAL" -gt "$Y2" ] && [ "$Y2" -gt "$Y1" ]
    then move$ARM 10 00 00 `echo "($YGOAL-$Y2)/($Y2-$Y1)*$SEC" | bc -l`
  elif [ "$Y2" -gt "$YGOAL" ] && [ "$YGOAL" -gt "$Y1" ]
    then move$ARM 20 00 00 `echo "($Y2-$YGOAL)/($Y2-$Y1)*$ALPHA*$SEC" | bc -l`
fi

#==================================================
# take another picture
#==================================================


#if (( ($YDONE==0) ))
#  then
#    /home/pi/work/lincoln/camera/takepic board.jpg
#    display board.jpg&

#==================================================
# determine final y-coord of red spot
#==================================================

#   convert board.jpg -fuzz $FUZZ\% -fill cyan +opaque $COLOR -colorspace GRAY -posterize 2 -fuzz 15% -fill $COLOR -opaque black board3.jpg
#   Y3=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | sort -n | awk ' { a[i++]=$2; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`
#   echo '*** y3='$Y3
#   display board3.jpg&
#fi

xdg-close
