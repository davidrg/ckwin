Date: 21 Mar 88 21:12:04 GMT
From: jeff@colgate.CSNET (Jeff Norden)
Subject: Using Kermit from shell mode

I'm not sure, but I think somebody asked about running kermit under shell
mode a while ago.  Anyway, here is some code that I find useful.  The result
is that I can log onto machines with primitive operating systems (VMS and
ATT system V :-), and still have the features of shell-mode available for
command history, etc.  It's also handy to be able to run a file transfer in
an emacs window.  The transfer is in the "background", but you can also
monitor or stop it easily.

The ^\ key is bound to a function for sending escape sequences to kermit,
and ^C^Q can be used to send any control characters needed thru to the
system you connect to.  A more serious problem is that some systems will not
recognize a ^J as an end-of-line character.  So LFD is bound to a new
function which acts just like CR usually does in shell-mode, but a ^M is
sent as an end-of-line.  Funcions are also provied to swap the bindings of
CR and LFD.  I've also included a filter which will clean out any ^M's or
^@'s that get typed at you, but I don't really recommend it.  There doesn't
seem to be an acceptably fast way to do this via emacs-lisp.  Invoking
kermit by the command " kermit | tr -d '\015' " seems to work better (on my
system anyway).

Put the code below in a file named  wherever/kermit-shell.el, and add

 (autoload 'kermit "wherever/kermit-shell.el" "" t) 

to your .emacs file.  Then use M-x kermit when you want to load in the new
functions and bindings.  ("kermit" is just a dummy function which causes the
file to be loaded.)

Here's how I've been using this setup.  We have several machines connected
thru a terminal switch.  If I want to connect to a unix system, then I use the
LFD key to talk to the switch, and ignore any ^M's in the buffer, and do a
"stty -echo nl" after I log in.  Then the only real difference from being in
local shell-mode is that it is you need to to type ^C^Q^C to send an
interrupt, and ^C^Q^Z for a stop signal, etc. (since ^C^C just generates a
local stop signal, which kermit ignores).  To connect to a VMS system, I use
a shell script to invoke kermit thru the tr filter, do "M-X cr-key", and
then tell VMS that I'm on a half-duplex terminal.

Please let me know if any bugs turn up.  Also, shell-send-input-cr is hacked
directly from the gnu source, so the GNU copyright and license apply.

				-Jeff Norden
				jeff@colgate.csnet
			Dept of Mathematics, Colgate University
			Hamilton, NY  13346.

--- cut here ----- do not fold, bend, or spindle ----- cut here -----------
;;Additions to shell mode for use with kermit, etc.
;;Feb 1988, Jeff Norden - jeff@colgate.csnet
;;This file is part of GNU Emacs, copying restrictions apply.
;;Please see the GNU Emacs General Public License for details.
;; Copyright (C) 1985, 1986, 1987 Free Software Foundation, Inc.

(require 'shell)

(defvar kermit-esc-char "\C-\\" "*Kermit's escape char")

(defun kermit-esc ()
"For sending escape sequences to a kermit running in shell mode."
 (interactive)
 (process-send-string 
   (get-buffer-process (current-buffer))
   (concat kermit-esc-char (char-to-string (read-char)))))

(defun send-char ()
"Send an arbitrary character to a program in shell mode."
 (interactive)
 (process-send-string 
   (get-buffer-process (current-buffer))
   (char-to-string (read-char))))

(define-key shell-mode-map "\C-\\" 'kermit-esc)
(define-key shell-mode-map "\C-cq" 'send-char)
;; extra bindings for folks suffering form ^S/^Q braindamage:
(define-key shell-mode-map "\C-c\\" 'kermit-esc)
(define-key shell-mode-map "\C-c\C-q" 'send-char)

(defun shell-send-input-cr ()
  "Same as shell-send-input, but a return (^M) is sent at the end 
 instead of a newline (^J)."
  (interactive)
  (end-of-line)
    (if (eobp)
	(progn
	  (move-marker last-input-start
	       (process-mark (get-buffer-process (current-buffer))))
	  (insert ?\n)
	  (move-marker last-input-end (point)))
    (beginning-of-line)
    (re-search-forward shell-prompt-pattern nil t)
    (let ((copy (buffer-substring (point)
				  (progn (forward-line 1) (point)))))
      (goto-char (point-max))
      (move-marker last-input-start (point))
      (insert copy)
      (move-marker last-input-end (point))))
    (condition-case ()
	(save-excursion
	  (goto-char last-input-start)
	  (shell-set-directory))
      (error (funcall shell-set-directory-error-hook)))
  (let ((process (get-buffer-process (current-buffer))))
    (process-send-region process last-input-start (- last-input-end 1))
    (process-send-string process "\r")
    (set-marker (process-mark process) (point))))

;; This is backwards of what makes sense, but ...
(define-key shell-mode-map "\n" 'shell-send-input-cr)

(defun cr-key ()
"For shell mode - set the return key to actually send a return and
the line feed key actually send a newline."
 (interactive)
 (define-key shell-mode-map "\r" 'shell-send-input-cr)
 (define-key shell-mode-map "\n" 'shell-send-input))

(defun nl-key () "The opposite of cr-key."
 (interactive)
 (define-key shell-mode-map "\n" 'shell-send-input-cr)
 (define-key shell-mode-map "\r" 'shell-send-input))

;; This filter works, but I don't especially recommend it.
(defun clean-filter (process string)
"A process filter which kills off ^M's and ^@'s."
 (set-buffer (process-buffer process))
 (let 
     ((firstpos (string-match "[^\C-@\r]+" string))
      (buffermark (process-mark process))
      (oldpt (point))
      (newstring '"")
      goback)
   (while firstpos
     (setq newstring 
	   (concat newstring (substring string firstpos (match-end 0))))
     (setq firstpos (string-match "[^\C-@\r]+" string (match-end 0))))
   (goto-char (marker-position buffermark))
   (setq goback (< oldpt (point)))
   (insert newstring)
   (set-marker buffermark (point))
   (if goback (goto-char oldpt))))

(defun clean-shell-on ()
"Add a filter to the process running in the current buffer so that
 null's and ^M's will not appear.
A better alternative is to pipe the program you want to run thru
 tr -d '\\015' "
 (interactive)
 (set-process-filter (get-buffer-process (current-buffer)) 'clean-filter))

(defun clean-shell-off ()
"Cancel a previous clean-shell-on command"
 (interactive)
 (set-process-filter (get-buffer-process (current-buffer)) nil))

(define-key shell-mode-map "\C-cC" 'clean-shell-on)
(define-key shell-mode-map "\C-cc" 'clean-shell-off)

(defun kermit () "Extensions to shell mode for using kermit"
 (interactive) (message "Kermit shell extensions have been loaded"))

------------------------------

