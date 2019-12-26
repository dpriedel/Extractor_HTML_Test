#!/bin/bash - 
#===============================================================================
#
#          FILE: create_shares_outstanding_test_file.sh
# 
#         USAGE: ./create_shares_outstanding_test_file.sh 
# 
#   DESCRIPTION: driver to help manually create training file for extracting
#                   shares outstanding
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 12/25/2019 09:05
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

for file_name in $(cat ./html_file_list.txt)
do
    echo $file_name
    qutebrowser "file:///${file_name}"
    read -n 1 -p Continue?
done


