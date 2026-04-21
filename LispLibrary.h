/*
  Fair dice LispLibrary - Version 1.0 - Dec 2025
  Hartmut Grawe - github.com/ersatzmoco - Dec 2025

  Licensed under the MIT license: https://opensource.org/licenses/MIT
*/


const char LispLibrary[] PROGMEM = R"lisplibrary(

#| Factory function: Create a two-dimensional array with size x/y. |#
(defun arrfact (xs ys) (eval (read-from-string (format nil "(make-array '(~a ~a) :element-type 'bit)" xs ys))))

#| Clear contents of two-dimensional array (fill it with 0). |#
(defun buf-clear (buf) 
	(let ((w (first (array-dimensions buf))) (h (second (array-dimensions buf))))
		(dotimes (k h)
			(dotimes (i w)
				(setf (aref buf i k) 0)
			)
		)
	)
)

#| "Draw" (copy) two-dimensional array to another two-dimensional array. |#
(defun buf-draw-arr (buf arr x y)
	(let ((bw (first (array-dimensions buf))) (bh (second (array-dimensions buf)))
		  (aw (first (array-dimensions arr))) (ah (second (array-dimensions arr)))	
		 )
		(dotimes (k ah)
			(dotimes (i aw)
				(when (and (< (+ y k) bh) (< (+ x i) bw))
					(setf (aref buf (+ x i) (+ y k)) (aref arr i k)) 
				)
			)
		)
	)
)

#| Create list of "particles": Each list element consists of two lists with 3 elements each. The first one
contains x/y/z coordinates of pixels "set" (= 1) in the two-dimensional array provided (z set to 0), the second
inner list is filled with randomized velocity vector values. |#
(defun make-particle-list (buf)
	(setf *particle-list* nil)
	(let ((w (first (array-dimensions buf))) (h (second (array-dimensions buf)))
		(dx 0) (dy 0) (dz 0)
		)
		(dotimes (k h)
			(dotimes (i w)
				(when (> (aref buf i k) 0)
					(if (<= (random 10) 4) (setf dx -1) (setf dx 1))
					(if (<= (random 10) 4) (setf dy -1) (setf dy 1))
					(if (<= (random 10) 4) (setf dz -2) (setf dz 1))
					(setf *particle-list* (append *particle-list* (list (list (list i k 20) (list dx dy dz)))))
				)
			)
		)
	)
)

#| Move particle positions according to their velocity vectors or inverse velocity vectors. |#
(defun move-particles (inverse)
	(dolist (p *particle-list*)
		(if inverse
			(setf (first p) 
				(list (- (first (first p)) (first (second p))) 
					(- (second (first p)) (second (second p)))
					(- (third (first p)) (third (second p)))
				)
			)
			(setf (first p) 
				(list (+ (first (first p)) (first (second p))) 
					(+ (second (first p)) (second (second p)))
					(+ (third (first p)) (third (second p)))
				)
			)
		)
	)
)

#| Bounce particles in virtual cube (reflecting them at virtual walls). Optionally increase particle energy
according to motion sensor values.  |#
(defun bounce-particles (&optional sensor)
	(let* ((fp nil) (sp nil) (newl nil)
		(x 0) (y 0) (z 0) (dx 0) (dy 0) (dz 0) (nx 0) (ny 0) (nz 0) 
		(sread (check-accel)) (sx (first sread)) (sy (second sread)) (sz (third sread)))

		#| Record whether motion of die has ceased (rounded sensor values are zero). |#
		(when (and (= 0 (round sx)) (= 0 (round sy)) (= 0 (round sy))) (setf *zero-motion* (1+ *zero-motion*)))

		#| Extract data of each particle and do calculations |#
		(dolist (p *particle-list*)
			(setf fp (first p))
			(setf sp (second p))
			(setf x (first fp))
			(setf y (second fp))
			(setf z (third fp))
			(setf dx (first sp))
			(setf dy (second sp))
			(setf dz (third sp))

			#| When sensor data is to be considered, randomize it a little and change velocity vector values. |#
			(when sensor
				(setf dx (+ dx sx))
				(setf dy (+ dy sy))
				(setf dz (+ dz sz))

				(setf dx (+ (* dx 0.9) (random 0.5)))
				(setf dy (+ (* dy 0.9) (random 0.5)))
				(setf dz (+ (* dz 0.9) (random 0.5)))
			)

			#| Move particles and check for "wall" collision |#
			(setf nx (round (+ x dx)))
			(when (> nx 15) (setf nx (- 15 dx)) (setf dx (- dx)))
			(when (< nx 0) (setf nx (- nx)) (setf dx (- dx)))

			(setf ny (round (+ y dy)))
			(when (> ny 17) (setf ny (- 17 dy)) (setf dy (- dy)))
			(when (< ny 0) (setf ny (- ny)) (setf dy (- dy)))

			(setf nz (round (+ z dz)))
			(when (> nz 10) (setf nz (- 10 dz)) (setf dz (- dz)))
			(when (< nz -10) (setf nz (- -10 dz)) (setf dz (- dz)))
			(when (= nz -10) (setf nz -9))

			(setf fp (list nx ny nz))
			(setf sp (list dx dy dz))
			(setf newl (append newl (list (list fp sp))))
		)
		(setf *particle-list* newl)
	)
)

#| Draw particles to canvas |#
(defun draw-particles ()
	(let ((x 0) (y 0) (z 0))
		(dolist (p *particle-list*)
			(setf x (first (first p)))
			(setf y (second (first p)))
			(setf z (third (first p)))

			(unless (or (< x 0) (< y 0) (< z -9) (> x 15) (> y 17) (> z 254))
				(canvas-draw-pixel (round x) (round y) (round (+ 15 z)))
			)
		)
	)
)

#| Calculate interpolation values for convergence of particles |#
(defun prep-converge-particles (steps)
	(let ((x 0) (y 0) (z 0)
		(mx 8) (my 9) (mz 10)
		(rlist nil))
		(dolist (p *particle-list*)
			(setf x (first (first p)))
			(setf y (second (first p)))
			(setf z (third (first p)))

			(setf rlist (append rlist (list (list 
						(first p) 
						(list (/ (- mx x) steps) (/ (- my y) steps) (/ (- mz z) steps)))
					))
			)
		)
		(setf *particle-list* rlist)
	)
)

#| Prepare six particles (random positions) for shake animation |#
(defun prep-six-particles ()
	(buf-clear *canvas-buf*)
	(dotimes (i 6)
		(setf (aref *canvas-buf* (random 16) (random 18)) 1)
	)
	(make-particle-list *canvas-buf*)
)

#| Prepare and execute shake animation of six particles. "steps" sets a maximum number of displayed motion
steps. If no sensor motion is detected anymore before that timeout, the animation is terminated. |#
(defun shake-particles (steps visible &optional del)
	(let ((d 10))
		(when del (setf d del))
		(prep-six-particles)
		#| Record how often the sonsor does not detect motion anymore |#
		(setf *zero-motion* 0)
		(canvas-set-frame *dframe*)
		(canvas-display-frame *sframe*)
		(dotimes (i steps)
			(bounce-particles t)
			(when visible
				(advance-frames)
				(draw-particles)
				(delay d)
			)
			(unless (< *zero-motion* 50) (return))
		)
		(canvas-display-frame *sframe*)
	)
)

#| Let particles diverge for a given number of animation steps. |#
(defun scramble-particles (steps visible inverse &optional del)
	(let ((d 10))
		(when del (setf d del))
		(dotimes (i steps)
			(move-particles inverse)
			(when visible
				(advance-frames)
				(draw-particles)
				(delay d)
			)
		)
		(canvas-display-frame *sframe*)
	)
)

#| Check motion sensor delta values (difference from last check) and double them to achieve a realistic visual result. |#
(defun check-accel ()
	(let* ((accel (accel-get-event)) 
			(a (first accel)) (b (second accel)) (c (third accel))
			(la (first *last-accel*)) (lb (second *last-accel*)) (lc (third *last-accel*))
			(da (* (- a la) 2)) (db (* (- a la) 2)) (dc (* (- a la) 2))
			)
		(setf *last-accel* accel)
		(list da db dc)
	)
)

#| Build fair result list for a given number of virtual "dice". |#
(defun build-dice-list (dice)
	(let* ((dlen (- (* 6 dice) (1- dice))) (rand (+ dice (random dlen))) (mylist nil))
		(loop
			(unless (search (list rand) mylist) (setf mylist (append mylist (list rand))))
			(unless (< (length mylist) dlen) (return))
			(setf rand (+ dice (random dlen)))
		)
		mylist
	)		
)

#| Wait for signicifcant shaking of cube. |#
(defun wait-shake (pin &optional wait)
	(let ((del 0) (rapidshake nil) (sens nil) (sensavg 0) (senscnt 0) (btnstate t) (lastbtnstate t) (btncnt 0))
		(when wait (setf del wait))
		(loop
			#| Advance pseudo random algorithm a random number of times (as long as we wait for a significant shake). |#
			(random 12)
			#| Significant motion detected: Report that and leave waiting loop. |#
			(when (> senscnt 15) (setf rapidshake t) (return))
			#| Dice mode button pressed long: Save this result and lave waiting loop. |#
			(when (> btncnt 50) (return))

			(setf sens (accel-get-event))
			#| Consider motion sensor data by averaging all three axes. |#
			(setf sensavg (abs (round (/ (+ (first sens) (second sens) (third sens)) 3))))
			(when (> sensavg 6) (setf senscnt (1+ senscnt)))
			#| Check dice mode button. |#
			(setf btnstate (digitalread pin))
			(if (eq btnstate lastbtnstate)
				(if btnstate 
					(setf btncnt 0)
					(setf btncnt (1+ btncnt))
				)
			)
			(setf lastbtnstate btnstate)
			(delay del)
		)
		rapidshake
	)
)

#| Double buffering: Direct drawing commands to next buffer and display current one.  (Cycle through 8 buffers.) |#
(defun advance-frames ()
	(canvas-display-frame *sframe*)
	(canvas-set-frame *dframe*)
	(canvas-clear)
	(setf *sframe* (1+ *sframe*))
	(setf *dframe* (1+ *dframe*))
	(when (> *sframe* 7) (setf *sframe* 0))
	(when (> *dframe* 7) (setf *dframe* 0))
)

#| Execute 3D animation (tumbling) of pixel image currently in buffer. |#
(defun tumble-image (steps reverse)
	(let ((w 0))
		(dotimes (ww steps)
			(if reverse 
				(setf w (- (1- steps) ww))
				(setf w ww)
			)
			(advance-frames)
			(canvas-transform-draw *canvas-buf* (list (* 18 w) 0 (* 12 w)) (list 0 0 (round (/ (* w w) 10))) -30 t)
			(delay 10)
		)
	)
	(when reverse (advance-frames) (canvas-draw-array *canvas-buf* t 10 0 0)
	)
)

#| Handle all checks, steps and animations for one die mode. |#
(defun one-die ()
	(buf-clear *canvas-buf*)
	(buf-draw-arr *canvas-buf* (aref *img-list* 18) 0 0)
	(tumble-image 30 t)
	(delay 2000)

	(make-particle-list *canvas-buf*)
	(setf *dice-list-1* nil)

	(loop
		#| Waiting for rapid shake terminated by dice mode pushbutton: Exit loop, start transition. |#
		(when (not (wait-shake 28 10)) (return))

		#| Cube shaking rapidly: Initiate and execute dice simulation with animation. |#
		(when (not *dice-list-1*) (setf *dice-list-1* (build-dice-list 1)))
		(scramble-particles 10 t nil) 
		(shake-particles 250 t 20) 
		(prep-converge-particles 20) 
		(scramble-particles 20 t nil)
		(buf-clear *canvas-buf*) 
		(buf-draw-arr *canvas-buf* (aref *img-list* (1- (pop *dice-list-1*))) 0 0) 
		(make-particle-list *canvas-buf*) 
		(prep-converge-particles 10) 
		(scramble-particles 10 nil nil)  
		(scramble-particles 9 t t)
		(canvas-draw-array *canvas-buf* t 10 0 0)
	)

	#| Transition to next dice mode. |#
	(tumble-image 30 nil)
)

#| Handle all checks, steps and animations for two dice mode. |#
(defun two-dice ()
	(buf-clear *canvas-buf*)
	(buf-draw-arr *canvas-buf* (aref *img-list* 19) 0 0)
	(tumble-image 30 t)
	(delay 2000)

	(make-particle-list *canvas-buf*)
	(setf *dice-list-1* nil)

	(loop
		#| Waiting for rapid shake terminated by dice mode pushbutton: Exit loop, start transition. |#
		(when (not (wait-shake 28 10)) (return))

		#| Cube shaking rapidly: Initiate and execute dice simulation with animation. |#
		(when (not *dice-list-1*) (setf *dice-list-1* (build-dice-list 2)))
		(scramble-particles 10 t nil) 
		(shake-particles 250 t 20) 
		(prep-converge-particles 20) 
		(scramble-particles 20 t nil)
		(buf-clear *canvas-buf*)
		(let* ((result (pop *dice-list-1*)) (tb-entry (nth (- result 2) *dice-table*)) 
				(len (length tb-entry)) (pair (nth (random len) tb-entry)))
			(buf-draw-arr *canvas-buf* (aref *img-list* (+ 5 (first pair))) 0 1)
			(buf-draw-arr *canvas-buf* (aref *img-list* (+ 5 (second pair))) 8 9)
		)
		(make-particle-list *canvas-buf*) 
		(prep-converge-particles 10) 
		(scramble-particles 10 nil nil)  
		(scramble-particles 10 t t)
	)

	#| Transition to next dice mode. |#
	(tumble-image 30 nil)
)

#| Handle all checks, steps and animations for symbol dice mode. |#
(defun sym-dice ()
	(buf-clear *canvas-buf*)
	(buf-draw-arr *canvas-buf* (aref *img-list* 20) 0 0)
	(tumble-image 30 t)
	(delay 2000)

	(make-particle-list *canvas-buf*)
	(setf *dice-list-1* nil)
	(setf *dice-list-2* nil)

	(loop
		#| Waiting for rapid shake terminated by dice mode pushbutton: Exit loop, start transition. |#
		(when (not (wait-shake 28 10)) (return))

		#| Cube shaking rapidly: Initiate and execute dice simulation with animation. |#
		(when (not *dice-list-1*) (setf *dice-list-1* (build-dice-list 1)))
		(when (not *dice-list-2*) (setf *dice-list-2* (build-dice-list 1)))
		(scramble-particles 10 t nil) 
		(shake-particles 250 t 20) 
		(prep-converge-particles 20) 
		(scramble-particles 20 t nil)
		(buf-clear *canvas-buf*) 
		(buf-draw-arr *canvas-buf* (aref *img-list* (+ 5 (pop *dice-list-1*))) 0 1)
		(buf-draw-arr *canvas-buf* (aref *img-list* (+ 11 (pop *dice-list-2*))) 8 9)
		(make-particle-list *canvas-buf*) 
		(prep-converge-particles 10) 
		(scramble-particles 10 nil nil)  
		(scramble-particles 10 t t)
	)

	#| Transition to next dice mode. |#
	(tumble-image 30 nil)
)

#| Setup prior to main loop: Set new random seed, create global variables and load images from SD card. |#
(defun setup ()
	(random-seed)
	(canvas-begin #x76 #x74)
	(pinmode 28 :input-pullup)
	(accel-begin)
	(accel-set-range 8)

	(defvar *dice-list-1* nil)
	(defvar *dice-list-2* nil)
	(defvar *dice-table* '(((1 1)) ((1 2) (2 1)) ((1 3) (3 1) (2 2)) ((1 4) (4 1) (2 3) (3 2)) ((1 5) (5 1) (2 4) (4 2) (3 3)) ((1 6) (6 1) (2 5) (5 2) (3 4) (4 3)) ((4 4) (2 6) (6 2) (3 5) (5 3)) ((4 5) (5 4) (3 6) (6 3)) ((5 5) (4 6) (6 4)) ((5 6) (6 5)) ((6 6))) )
	(defvar *particle-list* nil)
	(defvar *mode-list* '(1 2 3))
	(defvar *img-list* (make-array 21))
	(defvar *canvas-buf* (make-array '(16 18) :element-type 'bit))
	(defvar *last-accel* (accel-get-event))
	(defvar *zero-motion* 0)

	(defvar *dframe* 1)
	(defvar *sframe* 0)
	(canvas-clear-all)
	(canvas-set-frame *dframe*)

	#| Pre-load images. |#
	(dotimes (i 6) 
		(defvar setname_1 (concatenate 'string "1W_" (string (1+ i))))
		(defvar setname_2 (concatenate 'string "2W_" (string (1+ i))))
		(defvar setname_S (concatenate 'string "SW_" (string (1+ i))))

		(setf (aref *img-list* i) (arrfact 16 18))
		(setf (aref *img-list* (+ 6 i)) (arrfact 7 9))
		(setf (aref *img-list* (+ 12 i)) (arrfact 7 9))

		(load-mono (concatenate 'string setname_1 ".BMP") (aref *img-list* i))
		(load-mono (concatenate 'string setname_2 ".BMP") (aref *img-list* (+ 6 i)))
		(load-mono (concatenate 'string setname_S ".BMP") (aref *img-list* (+ 12 i)))
	)

	(dolist (m *mode-list*)
		(setf (aref *img-list* (+ 17 m)) (arrfact 16 18))
		(load-mono (concatenate 'string "MODE_" (string m) ".BMP") (aref *img-list* (+ m 17)))
	)
)

#| Main program called after startup of uLisp: Setup, then main loop. |#
(progn 
	(setup)

	#|main loop|#
	(loop
		(one-die)
		(two-dice)
		(sym-dice)
	)
)

)lisplibrary";