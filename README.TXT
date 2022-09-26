C-KERMIT 10.0 DEVELOPMENT

C-Kermit 10.0 Beta.05 27 September 2022

C-Kermit since about Alpha.05 could:
 . send packets that were slightly longer than the receiver could accept
 . send packets with illegal length fields
 . send long-format packets when this feature had not been negotiated

These problems should be fixed now, but verification is needed from
people who have old and/or 3rd-party Kermit implementations.
For greater detail see:

  https://www.kermitproject.org/ckupdates.html

which contains details of all the
previous developement versions, Alphas, and Betas since C-Kermit 9.0
was released in 2011.

