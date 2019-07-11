
#=======================================================================
#
# rotate toward red/green object combination
#
# Usage:
#
#  % fs.correct.x.rg 50 xx 1 <REDFUZZ> <GREENFUZZ>
#
#=======================================================================

ARM=$3
XGOAL=$1     # goal coordinates for red spot in image/gaze
YGOAL=$2     
XPPS=23      # very rough pixels per second in x direction (via shoulder rotation)
YPPS=33      # very rough pixels per vertical second in y direction (via elbow)
ALPHA=0.40   # compensate for gravity pulling elbow down

REDFUZZ=$4
GREENFUZZ=$5

#==================================================
# take picture, determine x-coord of red spot
#==================================================

/home/pi/work/lincoln/camera/takepic board.jpg
display board.jpg&

convert board.jpg -fuzz $REDFUZZ% -fill cyan +opaque red -colorspace GRAY -posterize 2 -fuzz 15% -fill red -opaque black board3.jpg

display board3.jpg&

XR=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`

convert board.jpg -fuzz $GREENFUZZ% -fill cyan +opaque green -colorspace GRAY -posterize 2 -fuzz 15% -fill green -opaque black board3.jpg

display board3.jpg&

XG=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`

X1=`echo $XR/2+$XG/2 | bc`

echo '*** xr='$XR
echo '*** xg='$XG
echo '*** x1='$X1

#==================================================
# rotate shoulder in direction of red spot
#==================================================

XDONE=0

if (( (($X1<=$XGOAL)&&(($XGOAL-$X1)<=2))||(($XGOAL<=$X1)&&(($X1-$XGOAL)<=2)) ))
  then move$ARM 00 00 00 0.1
       XDONE=1
       X2=X1
  elif [ "$X1" -lt "$XGOAL" ]
    then move$ARM 00 02 00 `echo "($XGOAL-$X1)/$XPPS" | bc -l`
         SEC=`echo "($XGOAL-$X1)/$XPPS" | bc -l`
  else move$ARM 00 01 00 `echo "($X1-$XGOAL)/$XPPS" | bc -l`
       SEC=`echo "($X1-$XGOAL)/$XPPS" | bc -l`
fi

#==================================================
# take another picture
#==================================================

if (( ($XDONE==0) ))
  then
    /home/pi/work/lincoln/camera/takepic board.jpg
    display board.jpg&
fi

#==================================================
# determine new x-coord of red spot
#==================================================

if (( ($XDONE==0) ))
  then
convert board.jpg -fuzz $REDFUZZ% -fill cyan +opaque red -colorspace GRAY -posterize 2 -fuzz 15% -fill red -opaque black board3.jpg

display board3.jpg&

XR=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`

convert board.jpg -fuzz $GREENFUZZ% -fill cyan +opaque green -colorspace GRAY -posterize 2 -fuzz 15% -fill green -opaque black board3.jpg

display board3.jpg&

XG=`convert board3.jpg txt:- | grep -v 'FFFFFF' | tr ',' ' ' | sed 's/:.*//' | sort -n | awk ' { a[i++]=$1; } END { x=int((i+1)/2); if (x < (i+1)/2) print int((a[x-1]+a[x])/2); else print a[x-1]; }'`

X2=`echo $XR/2+$XG/2 | bc`

echo '*** xr='$XR
echo '*** xg='$XG
echo '*** x2='$X2

fi

#==================================================
# figure out how much to move shoulder & move it
#==================================================

if (( (($XDONE==1)||(($X2<=$XGOAL)&&(($XGOAL-$X2)<=2))||(($XGOAL<=$X2)&&(($X2-$XGOAL)<=2))) ))
  then move$ARM 00 00 00 0.1
       XDONE=1
  elif [ "$X1" -gt "$X2" ] && [ "$X2" -gt "$XGOAL" ]
    then move$ARM 00 01 00 `echo "($X2-$XGOAL)/($X1-$X2)*$SEC" | bc -l`
  elif [ "$X1" -gt "$XGOAL" ] && [ "$XGOAL" -gt "$X2" ]
    then move$ARM 00 02 00 `echo "($XGOAL-$X2)/($X1-$X2)*$SEC" | bc -l`
  elif [ "$XGOAL" -gt "$X2" ] && [ "$X2" -gt "$X1" ]
    then move$ARM 00 02 00 `echo "($XGOAL-$X2)/($X2-$X1)*$SEC" | bc -l`
  elif [ "$X2" -gt "$XGOAL" ] && [ "$XGOAL" -gt "$X1" ]
    then move$ARM 00 01 00 `echo "($X2-$XGOAL)/($X2-$X1)*$SEC" | bc -l`
fi

xdg-close
