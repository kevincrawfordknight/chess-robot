
R=`echo $RANDOM%8 | bc`
if [ "$R" -eq 0 ] 
  then OPENING="e2e4"
elif [ "$R" -eq 1 ]
  then OPENING="d2d4"
elif [ "$R" -eq 2 ]
  then OPENING="d2d3"
elif [ "$R" -eq 3 ]
  then OPENING="b1c3"
elif [ "$R" -eq 4 ]
  then OPENING="g1f3"
elif [ "$R" -eq 5 ]
  then OPENING="g2g3"
elif [ "$R" -eq 6 ]
  then OPENING="c2c4"
else
  OPENING="f2f4"
fi

echo $OPENING
  
