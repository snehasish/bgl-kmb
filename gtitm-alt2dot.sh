#!/bin/bash

gtitmfile=$1
dotfile=`echo "${gtitmfile}.dot" `
echo "graph G{" > ${dotfile}
sed -n '/VERTICES/,/^$/p' ${gtitmfile} | tail -n +2 | head -n -1 | awk '{printf "%d;", $1}' >> ${dotfile}
sed -n '/EDGES/,/^$/p' ${gtitmfile} | tail -n +2 | awk '{printf "%d--%d [weight=%d];", $1, $2, $3 }' >> ${dotfile}
echo "}" >> ${dotfile}
