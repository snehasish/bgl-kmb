#!/bin/bash

inetfile=$1
dotfile=`echo "${inetfile}.dot"` 

nodes=`head -n 1 ${inetfile} | awk '{print $1}'`
links=`head -n 1 ${inetfile} | awk '{print $2}'`

echo "graph G{" > ${dotfile}
sed -n 2,$((${nodes}+1))p $inetfile | awk '{printf "%d;\n", $1}' >> ${dotfile}
tail -n +$((${nodes}+2)) $inetfile | awk '{printf "%d--%d [weight=%d];\n", $1, $2, $3}' >> ${dotfile}
echo "}" >> ${dotfile}

