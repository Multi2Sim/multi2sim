#!/bin/bash
########################### INPUTS ##########################
################ Number of Compute Units ####################
declare -i NumCUs=32
declare -i NumL2s=6
NumL1V=$NumCUs
NumL1S=`expr $NumCUs / 4`


############### Output Files ################################
si_config="si-config-$NumCUs"
mem_config="mem-si-$NumCUs"
net_config="net-si-$NumCUs"

#----------- Cache Args --------------------------
L1VectorGeoName="si-geo-vector-l1"
L1ScalarGeoName="si-geo-scalar-l1"
declare -i L1Sets=64
declare -i L1Assoc=4
declare -i BlockSize=64
declare -i L1Latency=1
L1Policy="LRU"
declare -i L1Ports=2

L2GeoName="si-geo-l2"
declare -i L2Sets=128
declare -i L2Assoc=16
declare -i L2Latency=10
L2Policy="LRU"
declare -i L2Ports=2

#------------- Net Args ---------------------------
net_name="si-net-l1-l2"

declare -i Inject_buffer=528
declare -i InternalBW=264
declare -i BW=72
declare -i PKT=0
declare -i FixDelay=8
vectors=$NumL1V
scalars=$NumL1S
Dests=$NumL2s

L2Nodes=`expr $Dests - 1`
Num_of_vn=`expr $vectors - 1`
Num_of_sn=`expr $scalars - 1`
Num_of_sw=`expr $vectors + $scalars + $Dests - 1`

###################### Launch Test ########################
L1Stester=`expr $NumCUs % 4`
if [ $L1Stester -ne 0 ]
then
	echo "Number of CUs are not divisible by 4"
	exit 1;
fi

###################### SI-Config ############################
echo "[ Device ]" > $si_config
echo "NumComputeUnits = $NumCUs" >> $si_config

##################### Mem-Config - Geometries ############################
L1cacheSize=`expr $L1Sets \\* $L1Assoc \\* $BlockSize`
echo ";------------L1 Vector : Cache size is $L1cacheSize " > $mem_config
echo "[CacheGeometry $L1VectorGeoName]" >> $mem_config
echo "Sets = $L1Sets" >> $mem_config
echo "Assoc = $L1Assoc" >> $mem_config
echo "BlockSize = $BlockSize" >> $mem_config
echo "Latency = $L1Latency" >> $mem_config
echo "Policy = $L1Policy" >> $mem_config
echo "Ports = $L1Ports" >> $mem_config
echo "" >> $mem_config

L1cacheSize=`expr $L1Sets \\* $L1Assoc \\* $BlockSize`
echo ";------------L1 Scalar : Cache size is $L1cacheSize " >> $mem_config
echo "[CacheGeometry $L1ScalarGeoName]" >> $mem_config
echo "Sets = $L1Sets" >> $mem_config
echo "Assoc = $L1Assoc" >> $mem_config
echo "BlockSize = $BlockSize" >> $mem_config
echo "Latency = $L1Latency" >> $mem_config
echo "Policy = $L1Policy" >> $mem_config
echo "Ports = $L1Ports" >> $mem_config
echo "" >> $mem_config

L2cacheSize=`expr $L2Sets \\* $L2Assoc \\* $BlockSize`
echo ";------------L2 Geometery : Cache size is $L2cacheSize " >> $mem_config
echo "[CacheGeometry $L2GeoName]" >> $mem_config
echo "Sets = $L2Sets" >> $mem_config
echo "Assoc = $L2Assoc" >> $mem_config
echo "BlockSize = $BlockSize" >> $mem_config
echo "Latency = $L2Latency" >> $mem_config
echo "Policy = $L2Policy" >> $mem_config
echo "Ports = $L2Ports" >> $mem_config
echo "" >> $mem_config

##################### Mem-Config - Si Vector L1s ############################
echo "L1 Vectors are $NumL1V"
L1VCounter=`expr $NumL1V - 1`
echo ";-------------------------------------L1 vectors" >> $mem_config
L2LowMods=""
L2Counter=`expr $NumL2s - 1`
for i in $(seq 0 $L2Counter)
do
	L2LowMods+="l2n$i "
done

for i in $(seq 0 $L1VCounter)
do
	echo "[Module si-vector-l1-$i]" >> $mem_config
	echo "Type = Cache" >> $mem_config
	echo "Geometry = $L1VectorGeoName" >> $mem_config
	echo "LowNetwork = $net_name" >> $mem_config
	echo "LowNetworkNode = l1v$i" >> $mem_config
	echo "LowModules = $L2LowMods" >> $mem_config
	echo "" >> $mem_config
done

##################### Mem-Config - Si Scalar L1s ###########################
echo "L1 Scalars are $NumL1S"
L1SCounter=`expr $NumL1S - 1`
echo ";-----------------------------------L1 scalars" >> $mem_config

for i in $(seq 0 $L1SCounter)
do
	echo "[Module si-scalar-l1-$i]" >> $mem_config
	echo "Type = Cache" >> $mem_config
	echo "Geometry = $L1ScalarGeoName" >> $mem_config
	echo "LowNetwork = $net_name" >> $mem_config
	echo "LowNetworkNode = l1s$i" >> $mem_config
	echo "LowModules = $L2LowMods" >> $mem_config
	echo "" >> $mem_config
done

##################### SI Compute Units  ###########################
echo "Compute Units are $NumCUs"
SICounter=`expr $NumCUs - 1`
echo ";-----------------------------------SI Compute Units" >> $mem_config

for i in $(seq 0 $SICounter)
do
	j=`expr $i / 4`
	echo "[Entry si-cu-$i]" >> $mem_config
	echo "Arch = SouthernIslands" >> $mem_config
	echo "ComputeUnit = $i" >>$mem_config
	echo "DataModule = si-vector-l1-$i" >>$mem_config
	echo "ConstantDataModule = si-scalar-l1-$j" >> $mem_config
	echo "" >> $mem_config
done

##################### L2 Cache Units  ###########################

echo ";---------------------------------L2 Cache Units " >> $mem_config
for i in $(seq 0 $L2Counter)
do
	echo "[Module l2n$i]" >> $mem_config
	echo "Type = Cache" >> $mem_config
	echo "Geometry = $L2GeoName" >> $mem_config
	echo "HighNetwork = $net_name" >> $mem_config
	echo "HighNetworkNode = l2n$i" >> $mem_config
	echo "LowNetwork = si-net-l2-$i-gm-$i" >> $mem_config
	echo "LowModules = si-gm-$i" >> $mem_config
	echo "AddressRange = ADDR DIV $BlockSize MOD $NumL2s EQ $i" >> $mem_config
	echo "" >> $mem_config
done

##################### Global Memory  ###########################

echo ";--------------------------------Main (Global) Memory " >> $mem_config
for i in $(seq 0 $L2Counter)
do
	echo "[Module si-gm-$i]" >> $mem_config
	echo "Type = MainMemory" >> $mem_config
	echo "HighNetwork = si-net-l2-$i-gm-$i" >> $mem_config
	echo "BlockSize = $BlockSize" >> $mem_config
	echo "Latency = 100" >> $mem_config
	echo "AddressRange = ADDR DIV $BlockSize MOD $NumL2s EQ $i" >> $mem_config
	echo "" >> $mem_config
done

###################### Internal Networks ######################

echo ";-------------------------------- Internal Networks " >> $mem_config
for i in $(seq 0 $L2Counter)
do
	echo "[Network si-net-l2-$i-gm-$i]" >> $mem_config
	echo "DefaultInputBufferSize = $Inject_buffer" >> $mem_config
	echo "DefaultOutputBufferSize = $Inject_buffer" >> $mem_config
	echo "DefaultBandwidth = $InternalBW" >> $mem_config
	echo "" >> $mem_config
done

######################## External Network ########################

echo ";--------------------------------------------Network" > $net_config
echo "[Network.$net_name]" >> $net_config
echo "DefaultInputBufferSize = $Inject_buffer" >>$net_config
echo "DefaultOutputBufferSize = $Inject_buffer" >> $net_config
echo "DefaultBandwidth = $BW" >>$net_config
echo "DefaultPacketSize = $PKT" >>$net_config
echo "" >> $net_config

echo ";--------------------------------------------------Nodes" >> $net_config

echo "VNodes :  $vectors"

for i in $(seq 0 $Num_of_vn)
do
echo "[Network.$net_name.Node.l1v$i]" >> $net_config
echo "Type = EndNode" >>$net_config
echo "" >>$net_config
done

echo "SNodes :  $scalars"

for i in $(seq 0 $Num_of_sn)
do
echo "[Network.$net_name.Node.l1s$i]" >> $net_config
echo "Type = EndNode" >>$net_config
echo "" >>$net_config
done

echo "L2Nodes :  $Dests"

for i in $(seq 0 $L2Nodes)
do
echo "[Network.$net_name.Node.l2n$i]" >> $net_config
echo "Type = EndNode" >>$net_config
echo "" >>$net_config
done

echo ";---------------------------------------------------Switches" >> $net_config

echo "Switches : $Num_of_sw + 1"

for i in $(seq 0 $Num_of_sw)
do
echo "[Network.$net_name.Node.s$i]" >> $net_config
echo "Type = Bus" >> $net_config
echo "FixDelay = $FixDelay" >> $net_config
echo "" >>$net_config
done

echo ";-----------------------------------------------------Links" >> $net_config

for x in $(seq 0 $Num_of_sw)
do
	for i in $(seq 0 $Num_of_vn)
	do
		if [ $x -ne $i ]
		then
			echo "[Network.$net_name.Link.s$x-l1v$i]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = s$x" >> $net_config
			echo "Dest = l1v$i" >> $net_config
			echo "" >> $net_config
		else
			echo "[Network.$net_name.Link.l1v$i-s$x]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = l1v$i" >> $net_config
			echo "Dest = s$x" >> $net_config
			echo "" >> $net_config
		fi
	done

	for i in $(seq 0 $Num_of_sn)
	do
		j=`expr $vectors + $i`
		if [ $x -ne $j ]
		then
			echo "[Network.$net_name.Link.s$x-l1s$i]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = s$x" >> $net_config
			echo "Dest = l1s$i" >> $net_config
			echo "" >> $net_config
		else
			echo "[Network.$net_name.Link.l1s$i-s$x]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = l1s$i" >> $net_config
			echo "Dest = s$x" >> $net_config
			echo "" >> $net_config
		fi
	done

	for i in $(seq 0 $L2Nodes)
	do
		j=`expr $vectors + $scalars + $i`
		if [ $x -ne $j ]
		then
			echo "[Network.$net_name.Link.s$x-l2n$i]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = s$x" >> $net_config
			echo "Dest = l2n$i" >> $net_config
			echo "" >> $net_config	
		else
			echo "[Network.$net_name.Link.l2n$i-s$x]" >> $net_config
			echo "Type = Unidirectional" >>$net_config
			echo "Source = l2n$i" >> $net_config
			echo "Dest = s$x" >> $net_config
			echo "" >> $net_config	
		fi

	done
done

