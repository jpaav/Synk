# Synk
A file synchronizing program with emphasis on functionality and settings
-----------------------------------------USB-----------------------------------------
USB is meant to be placed on a flashdrive along with an xml file. XML file format is 
shown in data.xml. If no argument is given, USB will default to data.xml as input. Any
"pair" entries greater than the value of "num" will be ignored. In addition, any paths
longer than 255 characters will be cut off at 255 characters.

Global Attributes:
num:	number of path pairs
buf:	buffer length for copying files (multiplied by 1024)
len:	maximum length for each path

Specific Attributes:
src:
dst:
ext:	all files with this extension will be ignored (excluding "." character).
			Separate multiple with semicolons. Currently only 3 concurant exts alowed.

