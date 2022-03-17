
mkdir packetPointer

robocopy headers packetPointer/headers
robocopy release packetPointer *.pdb
robocopy release packetPointer *.lib

pause