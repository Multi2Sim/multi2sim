#!/bin/bash
########################INPUTS#############################
declare -i NUM_COL=5
declare -i NUM_ROW=3
declare -i VC=1
declare -i Inject_buffer=528
declare -i BW=16
declare -i PKT=16
fl="net-torus"
NET="net0"

##########################################################
Nodes=`expr $NUM_COL \\* $NUM_ROW`
Num_of_n=`expr $Nodes - 1`
Num_of_sw=`expr $Nodes - 1`
COL=`expr $NUM_COL - 1`
ROW=`expr $NUM_ROW - 1`

echo ";--------------------------------------------Network" > $fl
echo "[Network.$NET]" >> $fl
echo "DefaultInputBufferSize = $Inject_buffer" >>$fl
echo "DefaultOutputBufferSize = $Inject_buffer" >> $fl
echo "DefaultBandwidth = $BW" >>$fl
echo "DefaultPacketSize = $PKT" >>$fl
echo "" >> $fl

echo ";--------------------------------------------------Nodes" >> $fl

echo "Nodes :  $Nodes"

for i in $(seq 0 $Num_of_n)
do
echo "[Network.$NET.Node.n$i]" >> $fl
echo "Type = EndNode" >>$fl
echo "" >>$fl
done

echo ";---------------------------------------------------Switches" >> $fl

echo "Switches :  $Nodes"

for i in $(seq 0 $Num_of_sw)
do
echo "[Network.$NET.Node.s$i]" >> $fl
echo "Type = Switch" >> $fl
echo "" >>$fl
done

echo ";-----------------------------------------------------Links" >> $fl
for i in $(seq 0 $Num_of_n)
do
	echo "[Network.$NET.Link.n$i-s$i]" >> $fl
	echo "Type = Bidirectional" >>$fl
	echo "Source = n$i" >> $fl
	echo "Dest = s$i" >> $fl
	echo "VC = $VC" >> $fl
	echo "" >> $fl
done

for i in $(seq 0 $ROW)
do
	for iter in $(seq 0 $COL)
	do
		V=`expr $i \\* $NUM_COL`
		X=`expr $iter + $V`
		Y=`expr $X + 1`
		if [ $iter -ne $COL ] 
		then
			echo "[Network.$NET.Link.s$X-s$Y]" >> $fl
			echo "Type = Bidirectional" >>$fl
			echo "Source = s$X" >> $fl
			echo "Dest = s$Y" >> $fl
			echo "VC = $VC" >> $fl
			echo "" >> $fl
		else
			echo "[Network.$NET.Link.s$X-s$V]" >> $fl
			echo "Type = Bidirectional" >>$fl
			echo "Source = s$X" >> $fl
			echo "Dest = s$V" >> $fl
			echo "VC = $VC" >> $fl
			echo "" >> $fl
		fi

		W=`expr $X + $NUM_COL`
		if [ $i -ne $ROW ] 
		then
			echo "[Network.$NET.Link.s$X-s$W]" >> $fl
			echo "Type = Bidirectional" >>$fl
			echo "Source = s$X" >> $fl
			echo "Dest = s$W" >> $fl
			echo "VC = $VC" >> $fl
			echo "" >> $fl
		else
			echo "[Network.$NET.Link.s$X-s$iter]" >> $fl
			echo "Type = Bidirectional" >>$fl
			echo "Source = s$X" >> $fl
			echo "Dest = s$iter" >> $fl
			echo "VC = $VC" >> $fl
			echo "" >> $fl
		fi
	done

done

