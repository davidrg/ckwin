/* Looks up the expected results for a vttest test in the specified file.
 * Results, if found, are returned by setting the %v and %n kermit variables.
 * While this can all be done from kermit-script (and there is an implementation
 * of this in vttest.ksc), doing it from REXX is a bit faster and more reliable.
 */

parse arg testid

filename = CKermit("return \m(checksum_file)")

testid = lower(testid)

do while lines(filename) = 1
    curlin = strip(linein(filename))
    if substr(curlin,1,1) <> "#" then do
        /* Data line */
        parse var curlin id checksum rest

        if id = testid then do
            /* Match found - return the details to K95 by setting variables */
            ".%v := """checksum""""
            ".%n := \ftrim("rest")"
            leave
        end
    end
end

rc = close(filename)
