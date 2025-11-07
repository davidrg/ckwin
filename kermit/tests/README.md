Kermit 95 Tests
---------------

## vttest.ksc - VTTEST automation
The `vtest.ksc` script automates as much of VTTEST as possible, stepping through
all of its menus running every test that doesn't:

 * require printer output
 * require keyboard input
 * require mouse input
 * affect only the window (title, position, state, etc)

For each test that is run, it records an image of the terminal buffer at the
completion of the test plus a checksum of the terminal buffer, resulting in an
HTML file containing nearly 8,000 image files. It takes a bit over half an hour
to run the full test.