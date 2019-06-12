;; filename: figlet.el
;; Kirby Files, 9/18/94.  kfiles@bbn.com
;; add font completion: James LewisMoss 27 Oct 2000, dres@debian.org
;; feel free to modify and distribute; there's not a lot here.
;; call M-x figlet-message to insert a large ascii text in your buffer.
;; Current option is to center text.  Feel free to change this if you'd
;; like.
(defvar fig-options "-c")

(setq save-eval-depth max-lisp-eval-depth)
(setq max-lisp-eval-depth 1000)

(defun collapse-lists (da-list)
  (cond ((stringp da-list) (list da-list))
        ((null da-list) nil)
        (t (append (collapse-lists (car da-list))
                   (collapse-lists (cdr da-list))))))

(defun generate-figlet-font-list (loc-list)
  "Generate a list of figlet fonts."
  (mapcar
   '(lambda (element)
      (cons element nil))
   (mapcar
    '(lambda (one-file)
       (let ((point (string-match ".flf" one-file)))
         (substring one-file 0 point)))
    (collapse-lists
     (mapcar
      '(lambda (dir-string)
         (directory-files (expand-file-name dir-string)
                          nil ".*\.flf"))
      loc-list)))))

;; replace this with "figlet -I2" to get the default font dir
(defvar fig-font-locations '("/usr/share/figlet"))

(defvar fig-font-list (generate-figlet-font-list fig-font-locations))

(defun figlet-message ()
  "Inserts large message of text in ASCII font into current buffer"
  (interactive "*")
  (setq str (read-from-minibuffer "Enter message: "))
  (setq font
        (completing-read "Which font: " fig-font-list nil t))
  ;; If the user enters nothing then font is empty string "".
  ;; Omit the -f option in that case, giving figlet's default font.
  (let ((args (append (and (not (equal font "")) (list "-f" font))
                      (list fig-options str))))
    (apply 'call-process "figlet" nil t t args))
  (message "Done printing"))


(setq max-lisp-eval-depth save-eval-depth)

(provide 'figlet)
