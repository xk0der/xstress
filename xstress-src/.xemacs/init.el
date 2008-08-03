;; always end a file with a newline
(setq require-final-newline t)

;; don't let `next-line' add new lines in buffer
(setq next-line-add-newlines nil)

;; enable wheelmouse support by default
(when window-system
  (mwheel-install))
