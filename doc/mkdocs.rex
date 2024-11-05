/*
 * This script updates pages in the Kermit 95 manual with the version numbers
 * and dates of the executing Kermit 95 instance, along with modified dates
 * and other things.
 *
 * It relies on external functions provided by Kermit 95, so can not be run
 * by a regular REXX interpreter - it can only be run from inside K95 with
 * its "rexx" command.
 *
 * This script also relies on the RexxRE and RegUtil external function
 * packages. RegUtil as standard with Regina REXX, while RexxRE must be built
 * from source separately.
 *
 * Parameters are passed to this script via Kermit macros:
 *  K-95> .file_in = "in\\input_file.html"
 *  K-95> .file_out = "out\\output_file.html"
 *  K-95> rexx call mkdocs.rex
 */

/* Load RexxRE. Because the Rexx interpreter lives as long as Kermit 95 does,
 * RexxRE will stay loaded after the end of this script so if we try to load
 * it every time we'll get errors. So before we load it, check to see if its
 * already loaded.
 */
if rxFuncQuery("reloadfuncs") = 1 then do
    say "RexxRE not loaded. loading..."
    rcc = rxfuncadd('reloadfuncs', 'rexxre', 'reloadfuncs')
    if rcc > 0 then do
      msg=""
      if rcc = 40 then msg="(not found)"
      say "Failed to load RexxRE, error:" rcc msg
      return 1
    end
    call reloadfuncs
    say "RexxRE loaded successfully"
end

/* And do the same for regutil */
if rxFuncQuery("sysloadfuncs") = 1 then do
    say "RegUtil not loaded. loading..."
    rcc = rxfuncadd('sysloadfuncs', 'regutil', 'sysloadfuncs')
    if rcc > 0 then do
      msg=""
      if rcc = 40 then msg="(not found)"
      say "Failed to load RegUtil, error:" rcc msg
      return 1
    end
    call sysloadfuncs
    say "RegUtil loaded successfully"
end


/* Get input and output file names. Because the names can contain spaces,
 * its easier to just pluck these out of Kermit rather than try and take
 * them as arguments.
 */
input_file = CKermit("return \m(file_in)")
output_file = CKermit("return \m(file_out)")
git_date = CKermit("return \m(git_date)")
dev_mode = CKermit("return \m(dev_mode)")

/* 1 = use https://kermitproject.org, 0 = use http://kermitproject.org */
use_https = CKermit("return \m(use_https)")

/* 1 = generate .html files, 0 = generate .htm files and fix up links */
use_html = CKermit("return \m(web_mode)")

/* This is a list of all html files that we'll replacing references to within
 * files */
html_files. = ''

if use_html = 0 then do
    /* If we're renaming html -> htm, then prepare the list of all html
     * files! */
    new_fn= CKermit("return \m(html_files)")
    do #=1  until  new_fn==''
    parse var new_fn html_files.# ',' new_fn
    end

    /* And update the name of *this* file we're processing */
    output_file = changestr(".html",output_file,".htm")
end

/*
say "input_file =" input_file
say "output_file =" output_file
say "git date =" git_date
*/

/* REXX doesn't allow returning stems, get_version_tags will create the tags
 * stem as a global, and populate it with information about the Kermit version
 * this script is running under. */
call get_version_tags

/* If Kermit can't supply gits date for this file, then use the existing
 * file date:
 */
if length(git_date) = 0 then do
    tags.last_update_iso = SysGetFileDateTime(input_file, 'm')
    parse var tags.last_update_iso f_date" "f_time
end
else do
    tags.last_update_iso = git_date
    parse var tags.last_update_iso f_date" "f_time" "f_tz
end

parse var f_date f_date_y"-"f_date_m"-"f_date_d

tags.last_update=left(date('W',f_date,'I'),3)" "left(date('M',f_date,'I'),3)" "f_date_d" "f_time" "f_date_y

tags.last_update_short = f_date_d" "date('M',f_date,'I')" "f_date_y

/*
say "Replacement Values"
say "------------------"
say "            Herald ($herald$):" tags.herald
say "               Version Number ($ver$):" tags.ver_num
say "                  Release ($ver-rel$):" tags.ver_rel
say "            Full version ($ver-full$):" tags.ver_full
say "            Version Date ($ver-date$):" tags.ver_date
say "Short Version date ($ver-short-date$):" tags.ver_short_date
say "    Version date ISO ($ver-date-iso$):" tags.ver_date_iso
say "-----"
say "     File last update ISO ($last_update_iso$):" tags.last_update_iso
say "             File last update ($last_update$):" tags.last_update
say " File last update short ($last_update_short$):" tags.last_update_short
say "-----"
*/

/* Ensure output file doesn't exist */
rcc = SysFileDelete(output_file)
/* rcc=0: success, rcc=2: doesn't exist */
if rcc > 2 then say "error deleting output file:" output_file "("rcc")"
if rcc = 1 then say "error deleting output file:" output_file "("rcc")"

/* Flags are:
 *       'x': use extended regular expressions;
 *       'c': respect case
 *       'n': newlines denote line-ends
 *       's': match variables to sub-expressions
 *       'd': re is the value delimiter (default)
 *       'v': re matches a var
 *       't': first var is a stem for all the fields
 */
tagre = ReComp('<!--\$([a-z\-]*)\$-->([^<]*)<!--/\$([a-z\-]*)\$-->', 'x')
replre = ReComp('\$!([a-z\-]*)\$', 'x')

do while lines(input_file) = 1

    input_string = linein(input_file)
    output_string = input_string

    /* Normalise links to kermitproject.org to use a specific scheme */
    if use_https = 2 then do
        output_string = changestr("http://kermitproject.org",output_string,"//kermitproject.org")
        output_string = changestr("http://www.kermitproject.org",output_string,"//www.kermitproject.org")
        output_string = changestr("https://kermitproject.org",output_string,"//kermitproject.org")
        output_string = changestr("https://www.kermitproject.org",output_string,"//www.kermitproject.org")
    end
    else if use_https = 1 then do
        output_string = changestr("http://kermitproject.org",output_string,"https://kermitproject.org")
        output_string = changestr("http://www.kermitproject.org",output_string,"https://www.kermitproject.org")
    end
    else if use_https = 0 then do
        output_string = changestr("https://kermitproject.org",output_string,"http://kermitproject.org")
        output_string = changestr("https://www.kermitproject.org",output_string,"http://www.kermitproject.org")
    end

    if use_html = 0 then do
        /* index.html becomes kermit95.htm */
        output_string = changestr('="index.html',output_string,'="kermit95.htm')

        /* replace any references to .html files with .htm files */
        do j=1 for #
            old_fn = html_files.j
            new_fn = substr(html_files.j,1,length(html_files.j)-1)
            /*say old_fn "-->" new_fn*/
            output_string = changestr('="'old_fn,output_string,'="'new_fn)
        end
    end

    do forever
        matched = ReExec(tagre, output_string, 'FIELDS')

        if matched < 1 then leave

        /* For debugging
        say fields.0 'fields'
        do i = 1 to fields.0
        say i '»'fields.i'«'
        end*/


        tag_name = fields.1
        fulltag="<!--$"tag_name"$-->"fields.2"<!--/$"fields.3"$-->"

        if tag_name <> fields.3 then do
            /* The begin and end tags don't match - just eliminate them. */
            say "Error: Mismatched tag:" fulltag
            tag_value = ""
        end
        else do
            tag_name = changestr("-",tag_name,"_")
            interpret "tag_value=tags."tag_name
        end

        output_string = changestr(fulltag,output_string,tag_value)
    end

    do forever
        matched = ReExec(replre, output_string, 'FIELDS')
        if matched < 1 then leave

        /* For debugging
        say fields.0 'fields'
        do i = 1 to fields.0
        say i '»'fields.i'«'
        end
        */

        tag_name = fields.1
        fulltag="$!"tag_name"$"

        tag_name = changestr("-",tag_name,"_")
        interpret "tag_value=tags."tag_name

        /*
        say " ReTag name:" tag_name
        say "ReTag value:" tag_value
        say "-----"
        say "  Input String:" output_string
        say "Tag to Replace:" fulltag
        */
        output_string = changestr(fulltag,output_string,tag_value)
        /*
        say "        Result:" output_string
        */

    end

    rcc = lineout(output_file, output_string)

end

call ReFree tagre
call ReFree replre

/* Set file timestamp */
/*
say "Output File:" output_file
say "     f_date:" f_date
say "     f_time:" f_time
*/
rcc = SysSetFileDateTime(output_file, f_date, f_time)

exit 0

get_version_tags: procedure expose tags.
    /* This sets tags. as a global as procedures aren't allowed to return stems.
     */

    input_file = CKermit("return \m(file_in)")
    git_date = CKermit("return \m(git_date)")
    dev_mode = CKermit("return \m(dev_mode)")

    tags.herald=CKermit("return \v(herald)")

    heraldre = ReComp('([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+) ([[:alpha:][:digit:].]+), ([[:alpha:]]+) +([[:digit:]]+) ([[:digit:]]+),', 'x')
    matched = ReExec(heraldre, tags.herald, 'FIELDS')
    call ReFree heraldre

    /* For debugging: output each of the things found in the herald
    say fields.0 'fields'
    do i = 1 to fields.0
    say i '»'fields.i'«'
    end*/

    tags.ver_num = fields.1                             /* $ver$ */

    if length(tags.ver_num) = 0 then do
        say "Failed to parse herald!"
        exit 0
    end


    /* TODO: set to DEV if in dev mode */
    if dev_mode = 0 then do
        tags.ver_rel = fields.2                             /* $ver_rel$ */
    end
    else
    do
        tags.ver_rel = 'DEV'
    end

    tags.ver_full = tags.ver_num tags.ver_rel           /* $ver-full$ */
    ver_month_short = fields.3
    ver_month_num = mshort_to_num(ver_month_short)
    ver_month = mnum_to_word(ver_month_num)
    ver_day = fields.4
    ver_year = fields.5

    tags.ver_short_date = ver_day ver_month_short ver_year       /* $ver-short-date$ */
    tags.ver_date = ver_day ver_month ver_year                   /* $ver-date$ */
    if ver_day < 10 then
        tags.ver_date_iso = ver_year"-"ver_month_num"-0"ver_day  /* $ver-date-iso$ */
    else
        tags.ver_date_iso = ver_year"-"ver_month_num"-"ver_day   /* $ver-date-iso$ */

    /* $ver_major$, $ver_minor$, $ver_rev$ */
    parse var tags.ver_num tags.ver_major"."tags.ver_minor"."tags.ver_rev

    tags.ver = tags.ver_major"."tags.ver_minor" "tags.ver_rel

    return

mshort_to_num: procedure
  mw.jan = '01'
  mw.feb = '02'
  mw.mar = '03'
  mw.apr = '04'
  mw.may = '05'
  mw.jun = '06'
  mw.jul = '07'
  mw.aug = '08'
  mw.sep = '09'
  mw.oct = '10'
  mw.nov = '11'
  mw.dec = '12'

  arg mon
  return mw.mon

mnum_to_word: procedure
  mw.01 = 'January'
  mw.02 = 'February'
  mw.03 = 'March'
  mw.04 = 'April'
  mw.05 = 'May'
  mw.06 = 'June'
  mw.07 = 'July'
  mw.08 = 'August'
  mw.09 = 'September'
  mw.10 = 'October'
  mw.11 = 'November'
  mw.12 = 'December'

  arg mon
  return mw.mon
