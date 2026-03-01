# Attributes
This outlines current use of character and line attributes including which bits
are unused and available for new features.

## Character Attributes
In ANSI emulations these are _mostly_ set via SGR - once the attribute is turned
on it affects every character received until the attribute is turned back off.

| Bit |  Value | Purpose        | Notes                                                                              |
|-----|--------|----------------|------------------------------------------------------------------------------------|
|     | 0x0000 | Normal         | No attributes set                                                                  |
| 1   | 0x0001 | Bold           |                                                                                    |
| 2   | 0x0002 | Underline      |                                                                                    |
| 3   | 0x0004 | Blink          |                                                                                    |
| 4   | 0x0008 | Reverse Video  |                                                                                    |
| 5   | 0x0010 | Invisible      |                                                                                    |
| 6   | 0x0020 | Protected      |                                                                                    |
| 7   | 0x0040 | Graphic        | _Possibly_ not really used for anything?                                           |
| 8   | 0x0080 | Dim            |                                                                                    |
| 9   | 0x0100 | Wyse Character | _Possibly_ WYSE emulations only?                                                   |
| 10  | 0x0200 | Erased         | Erased (empty/"null") cell. Formerly KUI_CHAR_ATTR_UPPER_HALF but never used.      |
| 11  | 0x0400 | _unused_       | Not currently used for anything. Formerly KUI_CHAR_ATTR_LOWER_HALF but never used. |
| 12  | 0x0800 | Italic         |                                                                                    |
| 13  | 0x1000 | Hyperlink      | cell is part of a hyperlink                                                        |
| 14  | 0x2000 | Crossed-out    |                                                                                    |
| 15  | 0x4000 | _unused_       | Not currently used for anything                                                    |
| 16  | 0x8000 | _unused_       | Not currently used for anything                                                    |

Future needs based on what other terminals support might include:
- _Four_ additional underline styles (Double, Dashed, Dotted, Wavy) - needs at
  least 3 bits
- Xterm double-underline (which can be set in combination with regular underline
  and possibly reported by DECRQSS)
- Superscript
- Subscript
- Overline (DECSGR)
- Slow Blink
- Marking escape sequences in debug mode. This would probably take a few
  attributes which would all be mutually exclusive with SGR attributes 
  (as SGR attributes aren't processed in debug mode). The purpose would be
  to allow interaction in debug mode: 
  - Is debug escape sequence (perhaps the unused underline style?)
  - Is control character (dotted underline?)
  

Combined these would need 8 bits. Right now there are three unused, and four
more could probably be freed up giving seven total by:
- Combining `Underline`, `Erased` and one of the unused bits into a single three
  bit field. Erased is mutually exclusive with all attributes, so storing it as
  a kind of underline style should be fine:
    - 0 - Not erased, not underlined
    - 1 - Underlined
    - 2 - Double Underlined
    - 3 - Dashed
    - 4 - Dotted
    - 5 - Wavy
    - 6 - _unused value_
    - 7 - Erased/null cell
- Repurposing the WYSE Character and Graphic Character attributes in VT emulation.

If more attribute bits are _really_ needed, in 24bit color builds there are four
bits in the color attribute that are unlikely to be needed for anything else.

Beyond that expanding attribute storage would be necessary.

## Line Attributes - 1 byte
These are per line, and they do scroll with the terminal. 

| Bit # | Value | Purpose           | Notes                            |
|-------|-------|-------------------|----------------------------------|
|     0 |  0x00 | Normal            | AKA Lower Half                   |
|     1 |  0x01 | Double Wide       |                                  |
|     2 |  0x02 | Double High       |                                  |
|     3 |  0x04 | Upper Half        |                                  |
|     4 |  0x08 | Wyse Protected    | _Possibly_ WYSE emulations only? |
|     5 |  0x10 | Wyse BG Bold      | _Possibly_ WYSE emulations only? |
|     6 |  0x20 | Wyse BG Invisible | _Possibly_ WYSE emulations only? |
|     7 |  0x40 | Wyse BG Dim       | _Possibly_ WYSE emulations only? |
|     8 |  0x80 | _unused_          | Not currently used for anything  |



## Color attributes (1 byte, 24bit color builds only)
These are per character, and are copied around to an extent like other character
attributes. Builds supporting 24-bit color have one byte for _color attribute_ 
storage:

| Bit # | Value | Purpose    | Notes                                  |
|-------|-------|------------|----------------------------------------|
|     1 |  0x01 | FG Indexed | FG colour is an indexed value, not RGB |
|     2 |  0x02 | BG Indexed | BG colour is an indexed value, not RGB |
|     3 |  0x04 | _unused_   |                                        |
|     4 |  0x08 | _unused_   |                                        |
|     5 |  0x10 | _unused_   |                                        |
|     6 |  0x20 | _unused_   |                                        |
|     7 |  0x40 | _unused_   |                                        |
|     8 |  0x80 | _unused_   |                                        |

Possible future needs:
- Underline color is set
- Underline color is indexed, not RGB

Using the remaining bits for non-color purposes would require special care as
K95 does treat color attributes specially. And of course those non-color uses
are only possible in 24-bit builds which are also currently Windows and GUI 
only.

## DECterm Ruled Lines
DECterm Ruled Lines ([#499](https://github.com/davidrg/ckwin/issues/499)), if
they're ever implemented, will require an additional four bits per character
cell but with the quirk that they're _not_ character attributes.

They're not turned on and off like other SGR attributes, rather they're drawn
on screen with an escape sequence. The only thing that erases them is the
matching erase lines escape sequence or having all cells that contain ruled
lines scroll naturally off the screen. Erasing character cells or even the 
entire screen doesn't impact ruled lines and DECRQSS doesn't report their 
presence.

Because of their special separate nature these would probably best be stored in 
a new kind of attribute byte separate from character attributes and line 
attributes. Trying to shoehorn them into normal character attributes (which
won't really have space for them anyway) is probably asking for weird bugs like
those DECterm has when it comes to ruled lines.