#!/bin/bash
########################INPUTS#############################
declare -i vectors=16
declare -i scalars=4
declare -i Dests=6
declare -i Inject_buffer=528
declare -i SwitchBW=72
declare -i BW=16
declare -i PKT=16
fl="net-si-ideal-$vectors"
NET="si-net-l1-l2"

# KEEP IN MIND THAT IF YOU WANT VARIABLE SIZES FOR BUFFERS OF
# EACH DIFFERENT LINK, YOU SHOULD ADD THEM INDIVIDUALLY SINCE 
# THIS SCRIPT DOESN'T SUPPORT THIS.

##########################################################
L2Nodes=`expr $Dests - 1`
Num_of_vn=`expr $vectors - 1`
Num_of_sn=`expr $scalars - 1`
Num_of_sw=`expr $vectors + $scalars + $Dests - 1`

echo ";--------------------------------------------Network" > $fl
echo "[Network.$NET]" >> $fl
echo "DefaultInputBufferSize = $Inject_buffer" >>$fl
echo "DefaultOutputBufferSize = $Inject_buffer" >> $fl
echo "DefaultBandwidth = $BW" >>$fl
echo "DefaultPacketSize = $PKT" >>$fl
echo "" >> $fl

echo ";--------------------------------------------------Nodes" >> $fl

echo "VNodes :  $vectors"

for i in $(seq 0 $Num_of_vn)
do
echo "[Network.$NET.Node.l1v$i]" >> $fl
echo "Type = EndNode" >>$fl
echo "" >>$fl
done

echo "SNodes :  $scalars"

for i in $(seq 0 $Num_of_sn)
do
echo "[Network.$NET.Node.l1s$i]" >> $fl
echo "Type = EndNode" >>$fl
echo "" >>$fl
done

echo "L2Nodes :  $Dests"

for i in $(seq 0 $L2Nodes)
do
echo "[Network.$NET.Node.l2n$i]" >> $fl
echo "Type = EndNode" >>$fl
echo "" >>$fl
done

echo ";---------------------------------------------------Switches" >> $fl

echo "Switches : $Num_of_sw + 1"

for i in $(seq 0 $Num_of_sw)
do
echo "[Network.$NET.Node.s$i]" >> $fl
echo "Type = Switch" >> $fl
echo "BandWidth = $SwitchBW" >> $fl
echo "" >>$fl
done

echo ";-----------------------------------------------------Links" >> $fl

for x in $(seq 0 $Num_of_sw)
do
	for i in $(seq 0 $Num_of_vn)
	do
		if [ $x -ne $i ]
		then
			echo "[Network.$NET.Link.s$x-l1v$i]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = s$x" >> $fl
			echo "Dest = l1v$i" >> $fl
			echo "" >> $fl
		else
			echo "[Network.$NET.Link.l1v$i-s$x]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = l1v$i" >> $fl
			echo "Dest = s$x" >> $fl
			echo "" >> $fl
		fi
	done

	for i in $(seq 0 $Num_of_sn)
	do
		j=`expr $vectors + $i`
		if [ $x -ne $j ]
		then
			echo "[Network.$NET.Link.s$x-l1s$i]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = s$x" >> $fl
			echo "Dest = l1s$i" >> $fl
			echo "" >> $fl
		else
			echo "[Network.$NET.Link.l1s$i-s$x]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = l1s$i" >> $fl
			echo "Dest = s$x" >> $fl
			echo "" >> $fl
		fi
	done

	for i in $(seq 0 $L2Nodes)
	do
		j=`expr $vectors + $scalars + $i`
		if [ $x -ne $j ]
		then
			echo "[Network.$NET.Link.s$x-l2n$i]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = s$x" >> $fl
			echo "Dest = l2n$i" >> $fl
			echo "" >> $fl	
		else
			echo "[Network.$NET.Link.l2n$i-s$x]" >> $fl
			echo "Type = Unidirectional" >>$fl
			echo "Source = l2n$i" >> $fl
			echo "Dest = s$x" >> $fl
			echo "" >> $fl	
		fi

	done
done

