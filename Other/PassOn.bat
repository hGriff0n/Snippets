:: Setting the "open with" program automatically sets the file's icon that windows displays
:: Unfortunately, this icon is tied to the program that is used to open the file
:: This causes several different file types to have the same file icon, which can be hard/impossible to unravel
:: This batch file is used to get around the limitations of file icon associations
:: By using a BatToExe converter, it's possible to create many instances of this forwarding script
:: Then tie each instance to a separate file type, allowing the icon to be changed freely without contamination

:: Open with sends the file to open as the first argument to the program
:: Take the file to open and forward it to Visual Code (or any program of your choice)
start /b code %1

:: Prevent the command prompt from staying up (Also choose a "run invisible" option in the converter)
exit 0
