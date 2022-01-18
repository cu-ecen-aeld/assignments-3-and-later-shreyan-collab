#!/bin/bash
# @File name :finder.sh
# @brief     :Finder script to find the occurence of a string within
#	       a directory
# @author    :Shreyan Prabhu 01/14/2022
# @References: 1) https://ryanstutorials.net/bash-scripting-tutorial/bash-variables.php 
#	       2) https://ryanstutorials.net/bash-scripting-tutorial/bash-if-statements.php

errormessage="ERROR: Invalid Number of arguments\nTotal number of arguments must be 2.\nThe order of arguments should be\n1.Path of the directory\n2.String to be searched within the directory"

if [ ! $# -eq 2 ]		#Checking if number of arguments is 2
then
	echo -e $errormessage
	exit 1
fi

directory=$1			#Checking if the directory is valid
if [ ! -d "$directory" ]
then 
	echo "It is not a directory"
	exit 1
fi

word=$2
cd $directory
totallines=$(grep -r $word . | wc -l)	#Checking the entire directory
totalfiles=$(find -type f | wc -l)    #Using pipe to count files
echo "The number of files are $totalfiles and the number of matching lines are $totallines"
