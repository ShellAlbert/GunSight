;;; 50figlet.el -- debian emacs setups for figlet

(if (not (file-exists-p "/usr/share/emacs/site-lisp/figlet.el"))
    (message "figlet removed but not purged, skipping setup")

  (autoload 'figlet-message "figlet"
    "Inserts large message of text in ASCII font into current buffer" t))
