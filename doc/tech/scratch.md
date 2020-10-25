

[cities]

# city 2
coords (16 34)
wall_bounds (10 20 30 50)
devs.type [1, 2, 3, 4, 1, 2,]
    .pos [(1 2), (2 3)]
    .tags [] [] [] []

font 12px Helvetica|Arial|sans-serif
fonts [12px Helvetica|Arial|sans-serif, 
       13px Helvetica|Arial]
devs [1 (1 2) 0, 2 (3 4) 0]
obj.foo 12
   .bar.bar2 "weird"
      ..zeta true
	   ..sys.w 22
	      ...y 22
	   ..good quick
f2.devs.x 12
       ..y 12
   .devs.y 12
obj.bar 2

obj
   .foo 12
   .bar
      ..bar2 "weird"
      ..zeta true
	   ..sys
         ...w 22
	      ...y 22
	   ..good quick
   .fez false



# city :4
coords = (16 34)
wall_bounds = (10 20 30 50)
devs.type = [1 2 3 4 1 2]
devs.pos = [(1 2) (2 3)]
devs.tags = [ [] [] [] [] ]
obj.foo = 12
   .bar.bar2 = "weird"
      ..zeta = true
obj.devs.x = 12
       ..y = 12
   .devs.y = 12
obj.bar = 2



# city foo 0
coords (16 34)
wall_bounds (10 20 30 50)
dev {type 0, coords (0 0)}
dev {type 1, coords (0 0)}
dev {type 1, coords (0 0)}
dev {type 2, coords (0 0)}



[structures]

# (0, 0)
:@dev [(0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0), (0 0)]


# (-1, -1)


[terrain]

# (0, 0)
`______^^^^^TTTTTTTTTTTTTTT______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
`TTTTTTTTTTTTTTT-----------______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
`~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
`~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
`~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
`DDDDDDDDDDDDDDDDDDDDDDDDDD______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT

# (1, 1)

```
______^^^^^TTTTTTTTTTTTTTT______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
TTTTTTTTTTTTTTT-----------______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
~~~~~~~~~~~~~~~~~~~~~~~~~~______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
DDDDDDDDDDDDDDDDDDDDDDDDDD______^^^^^TTTTTTTTTTTTTTTTTTTTTTTTTTT
```

# (2, 2)
|______^^^^^TTTTTTTTTTTTTTT
|TTTTTTTTTTTTTTT-----------
|~~~~~~~~~~~~~~~~~~~~~~~~~~
|~~~~~~~~~~~~~~~~~~~~~~~~~~
|~~~~~~~~~~~~~~~~~~~~~~~~~~
|DDDDDDDDDDDDDDDDDDDDDDDDDD



