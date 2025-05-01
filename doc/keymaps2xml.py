# This is the script originally used to generate the keymap tables
# in ctlseqs.xml. It's here if it should ever need to be called upon again.

# file to read
file = "C:\\dev\\ckwin\\kermit\\k95\\dist\\keymaps\\default.ksc"

# These terminals aren't currently included in ctlseqs.xml as they're not handled separate from the ANSI/VT escape
# sequence parser. They'll probably eventually be documented in separate files some day (as ctlseqs.xml is already
# *massive*), so they're not included in the outputs from this script currently.
excluded_terminals = ['wy30', 'wy60', 'wy160', 'dg210', 'dg200', 'dg217', 'adm5', 'tvi910+', 'tvi950', 'hpterm',
                      'hp2621a', 'vc404', 'qnx', 'hz1500', 'avatar/0+', 'adm3a', 'wy50', 'tvi925']

# These are ordered to try and use colspan as much as possible to reduce table width. As a side effect, terminals with
# the most similar key maps tend to be clustered together.
std_terminals = [ 'heath19', 'vt52', 'vt100', 'vt102', 'vt220', 'vt320', 'vt320pc', 'vt220pc', 'wy370', 'ansi-bbs',
                  'vip7809', 'sni-97801', 'scoansi', 'at386', 'linux', 'beterm', 'qansi',  'tty', 'annarbor', 'ibm3151',
                  'ba80', 'aixterm', 'hft', 'sun' ]

# Special keyboard modes
modes = ['wp', 'emacs', 'hebrew', 'russian']

def get_keymap_for_terminals(fn, terminals):
    """
    Reads key definitions for the listed terminals from the specified filename.

    :param fn: pathname for default.ksc
    :param terminals: List of terminals to get keymaps for
    :return: keymap dict, and a set of kverbs seen
    """
    keys = dict()

    kverbs = set()

    with open(fn, 'r') as f:
        for line in f:

            terminal = None
            literal = False
            key = None
            action = None
            comment = None

            if line.startswith(";"): continue
            if "if NOT eq" in line: continue
            if "set mskermit" in line: continue
            if "set key clear" in line: continue

            if ';' in line:
                try:
                    bits = line.split(';')
                    line = bits.pop(0)
                    comment = ';'.join(bits)
                    comment = comment.strip()
                except:
                    print(line)
                    break

            line = line.strip()
            if line == '':
                # Just an empty line / line with a comment. Not interesting.
                continue

            # Don't care about the set terminal key command stuff - get rid of it
            line = line.replace('set terminal key ', '').strip()

            # We should now be left with something like:
            #       aixterm \123 \Kuparr
            # or:   aixterm /literal \882 \{27}[015q

            # Pluck out the terminal
            ind = line.index(' ')
            terminal = line[:ind]
            line = line[ind:].strip()

            if terminal not in terminals: continue   # Not interested in this terminal

            if line.startswith("clear"): continue   # set terminal key x clear...

            # Consume the /literal if it's there
            if line.startswith("/literal"):
                literal = True
                line = line.replace("/literal", "").strip()

            ind = line.index(' ')
            key = line[:ind]
            action = line[ind:].strip()

            if key is None or action is None or terminal is None: continue

            key = key.replace("<","&lt;").replace("&", "&amp;")
            comment = comment.replace("<", "&lt;").replace("&", "&amp;")
            action = action.replace("<", "&lt;").replace("&", "&amp;")

            if key not in keys:
                keys[key] = dict()
                keys[key]["comment"] = comment
                keys[key]["terminal"] = dict()
                keys[key]["terminal"][terminal] = {
                    "literal": literal,
                    "action": action
                }
            else:
                keys[key]["terminal"][terminal] = {
                    "literal": literal,
                    "action": action
                }

            if action.startswith("\K"):
                kverbs.add(action)

    return keys, kverbs

def output_keys_table(fn, keys, terminals):
    """
    Writes out something that looks like but isn't quite an HTML table, suitable to be pasted into ctlseqs.xml

    The table contains Key names and scan codes down the left, terminal types along the top, and key definitions in
    the middle

    :param fn: Output filename
    :param keys: Key definitions from get_keymap_for_terminals
    :param terminals: Terminal columns in the order they should be output
    :return:
    """
    #sorted_keys = sorted(keys.keys(), key=lambda x: keys[x]["comment"])
    sorted_keys = sorted(keys.keys(), key=lambda x: int(x[1:]))

    with open(fn, "w") as f:
        def write_html(h):
            print(h)
            f.write(h + "\n")

        write_html("""
        <table>
            <tr>
                <th>Key</th>
                <th>Key Code</th>""")
        for t in terminals:
            write_html("                <th>{0}</th>".format(t))
        write_html("            </tr>")

        log = False

        for k in sorted_keys:
            tcols = ""
            termkeys = keys[k]["terminal"]

            col = ""
            count = 0

            def do_append():
                nonlocal tcols, count, col
                if count == 1:
                    if col == "":
                        tcols += '                <td style="not-mapped"/>\n'
                    else:
                        tcols += "                <td>{0}</td>\n".format(col)
                elif count > 1:
                    if col == "":
                        tcols += '                <td style="not-mapped" colspan="{0}"/>\n'.format(count)
                    else:
                        tcols += '                <td colspan="{0}">{1}</td>\n'.format(count, col)
                count = 0
                col = ""

            for t in terminals:
                if log: print("Term: {0}".format(t))
                if t in termkeys:
                    lit = termkeys[t]["literal"]
                    act = termkeys[t]["action"]
                    if log: print("\tlit: {0}".format(lit))
                    if log: print("\tact: {0}".format(act))

                    if act.startswith("\K"):
                        # Its a kverb!
                        defn = '<a href="#kverb_' + act[1:] + '">' + act + '</a>'
                    else:
                        defn = act

                    if lit: defn = "<tt>" + defn + "</tt>"

                    if count == 0:
                        # Nothing to colspan further
                        col = defn
                        count += 1
                        if log: print("New definition: [{0}]={1}".format(col, count))
                    elif col == defn:
                        # Current terminals definition is the same as the previous. Add to the colspan!
                        count += 1
                        if log: print("Definition Matches: [{0}]={1}".format(col, count))
                    else:
                        # Current terminals definition is different. End the colspan
                        if log: print("End Definition: [{0}]={1}".format(col, count))
                        do_append()

                        # And start a new one
                        col = defn
                        count = 1
                        if log: print("New Definition: [{0}]={1}".format(col, count))

                else:
                    if col == "" and count > 0:
                        count += 1
                        if log: print("Definition Matches: [{0}]={1}".format(col, count))
                    else:
                        do_append()
                        col = ""
                        count = 1
                        if log: print("New Definition: [{0}]={1}".format(col, count))

            do_append()

            write_html("""            <tr>
                <td>{0}</td>
                <td>{1}</td>
{2}            </tr>""".format(keys[k]["comment"], k, tcols))

        write_html("</table>")

def output_kverbs_sections(file, kverbs):
    """
    Outputs *EMPTY* kverb sections suitable to be pasted into ctlseqs.xml

    :param file: Output filename
    :param kverbs: List of kverbs to output
    :return:
    """

    kverbs_sorted = sorted(kverbs)
    with open(file, "w") as f:
        def write_html(h):
            print(h)
            f.write(h + "\n")

        write_html('    <section id="kverbs" toc="true" role="text" title="Keyboard Verbs">')
        for kv in kverbs_sorted:
            write_html("""
                <section id="{1}" role="text" mnemonic="{0}" title="">
                    <p></p>
                </section>""".format(kv, "kverb_" + kv[1:]))
        write_html('    </section>')

# Read the keymaps for the terminals covered by ctlseqs.xml
keys_std, kverbs_std = get_keymap_for_terminals(file, std_terminals)
# And also for the special keyboard modes, which we'll include in ctlseqs.xml
keys_modes, kverbs_modes = get_keymap_for_terminals(file, modes)

# Output tables for the terminals and modes
output_keys_table("keys-std.html", keys_std, std_terminals)
output_keys_table("keys-modes.html", keys_modes, modes)

# And also output a template kverbs section for us to fill out
all_kverbs = kverbs_std.union(kverbs_modes)
output_kverbs_sections("kverbs-std.html", all_kverbs)