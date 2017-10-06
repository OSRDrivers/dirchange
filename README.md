# Directory Change Notification Watcher #

This utility watches all of the directory change notifications that fire on the specified directory path and all subdirectories. 

## Building ##

The provided solution builds in Visual Studio 2017

## Usage ##

    dirchange <path>

## Example Output ##

    E:\depot\tools\dirchange\x64\Debug>dirchange e:\
    Modifications to e:\ will be shown as they happen
    NewEntry:
    	Action: 3 (FILE_ACTION_MODIFIED)
    	Name: foo.txt
    NewEntry:
    	Action: 3 (FILE_ACTION_MODIFIED)
    	Name: foo.txt
    NewEntry:
    	Action: 4 (FILE_ACTION_RENAMED_OLD_NAME)
    	Name: foo.txt
    NewEntry:
    	Action: 5 (FILE_ACTION_RENAMED_NEW_NAME)
    	Name: bar.txt