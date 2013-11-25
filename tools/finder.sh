#!/bin/sh
#a simple shell to find
#by fux 13/11/25
content='0'
while : 
do
echo "============================="
echo "Enter what you want to find:"
read content
if [ -n "$content" ]; then
	find .. | xargs grep -n "$content"
else
	break;
fi
done