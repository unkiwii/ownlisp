; Atoms
(def nil {})
(def true 1)
(def false 0)

; Function definition (global)
(def defn (\ {f b} {
  def (head f) (\ (tail f) b)
}))

; Curry functions
(defn {curry f l} {
  eval (join (list f) l)
})

(defn {uncurry f : xs} {
  f xs
})

; Do many things in sequence
(defn {do : l} {
  if (= l nil) {
    nil
  } {
    last l
  }
})

; Open new scope
(defn {let b} {
  ((\ {_} b) ())
})

; Logical operatos
(defn {not x}   {- 1 x})
(defn {and x y} {* x y})
(defn {or x y}  {+ x y})

; List functions
(defn {fst l} {
  eval (head l)
})

(defn {snd l} {
  eval (head (tail l))
})

(defn {trd l} {
  eval (head (tail (tail l)))
})

(defn {len l} {
  if (= l nil) {
    0
  } {
    + 1 (len (tail l))
  }
})

(defn {nth n l} {
  if (= n 0) {
    fst l
  } {
    nth (- n 1) (tail l)
  }
})

(defn {last l} {
  nth (- (len l) 1) l
})

(defn {take n l} {
  if (= n 0) {
    nil
  } {
    join (head l) (take (- n 1) (tail l))
  }
})

(defn {drop n l} {
  if (= n 0) {
    l
  } {
    drop (- n 1) (tail l)
  }
})

(defn {split n l} {
  list (take n l) (drop n l)
})

(defn {elem x l} {
  if (= l nil) {
    false
  } {
    if (= x (fst l)) {
      true
    } {
      elem x (tail l)
    }
  }
})

; High-order functions
(defn {map f l} {
  if (= l nil) {
    nil
  } {
    join (list (f (fst l))) (map f (tail l))
  }
})

(defn {filter f l} {
  if (= l nil) {
    nil
  } {
    join (if (f (fst l)) {head l} {nil}) (filter f (tail l))
  }
})

(defn {foldl f z l} {
  if (= l nil) {
    z
  } {
    foldl f (f z (fst l)) (tail l)
  }
})

; Conditional functions
(defn {select : cs} {
  if (= cs nil) {
    error "No selection found"
  } {
    if (fst (fst cs)) {
      snd (fst cs)
    } {
      curry select (tail cs)
    }
  }
})

; base case for select
(def otherwise true)

(defn {case x : cs} {
  if (= cs nil) {
    error "No case found"
  } {
    if (= x (fst (fst cs))) {
      snd (fst cs)
    } {
      curry case (join (list x) (tail cs))
    }
  }
})

; Misc
(defn {flip f a b} {f b a})
(defn {comp f g x} {f (g x)})

(defn {for from to block} {
  let {
    do
      (if (> from to)
        { := {step} -1 }
        { := {step} 1 })
      (:= {_for} (\ {from to} {
        if (= from to)
          { nil }
          { do
            (block from)
            (_for (+ from step) to)
          }
      }))
      (_for from to)
  }
})
