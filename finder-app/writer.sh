#!/bin/bash
# @File name :finder.sh
# @brief     :Writer script to create a file in specific directory and the 
# @author    :Shreyan Prabhu 01/14/2022
# @References: 1) https://ryanstutorials.net/bash-scripting-tutorial/bash-variables.php 
#	       2) https://ryanstutorials.net/bash-scripting-tutorial/bash-if-statements.php


errormessage="ERROR: Invalid Number of arguments\nTotal number of arguments must be 2.\nThe order of arguments should be\n1.File path \n2.String to be written to the path"

if [ ! $# -eq 2 ]		 #Checking if number of arguments is 2
then
	echo -e $errormessage
	exit 1
fi

fullpath="$1"
directory="$(dirname "${fullpath}")"	#Extracting directory from path
filename="$(basename "${fullpath}")"	#Extracting filename from path
if [ ! -d "$directory" ]		#checking is directory is present
then 
	mkdir -p $directory		

fi

touch $fullpath
cd $directory
if [ ! -f "$fullpath" ]
then
	echo "File did not exist"	#Checking if the file is created
	exit 1
fi

word=$2
echo $word > $filename
if [ ! $? -eq 0 ]			#Check if last command is executed
then 
	echo "The input string is not written to the file"
	exit 1
fi
