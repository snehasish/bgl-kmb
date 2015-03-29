#!/bin/bash

britefile=$1
dotfile=`echo "${britefile}.dot" 
echo "graph G{" > ${dotfile}
sed -n '/Nodes:/,/^$/p' ${britefile} | tail -n +2 | sed '/^$/d' | awk '{printf "%d;\n",$1}' >> ${dotfile}
sed -n '/Edges:/,/^$/p' ${britefile} | tail -n +2 | sed '/^$/d' | awk '{printf "%d--%d [weight=%.2f];\n",$2,$3,$4}' >> ${dotfile}
echo "}" >> ${dotfile}
