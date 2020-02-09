Example brainfuck script

- anchor mem addr and so we don't underflow the stack since there
is no protection 

On compiled versions you can read the stack if you want

set first 28 to 100
>++++ ++++ ++++ ++++ ++++ +++
[
	[>]
	>++++ ++++ ++
	[-
	<
	++++ ++++ ++
	>
	]
	<
	+[-<+]-
>-
]

loop to set newline char
++++ ++++ +
[-
	>---- ---<
]
all of this is just going through the memory and adjusting the values for the string
++++ ++++ ++
>----
>+++ 
>++++ ++++ ++ 
>++++ + 
>>--- 
>++++ ++++ +++ 
>++++ ++++ 
>++++ ++++ ++ 
>++++ ++++ ++++ ++++ +++ 
>++++ ++++ +++ 
>>---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
>++++ ++++ ++++ ++ 
>++++ ++++ +++ 
>++ 
>---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- space
>++++ ++++ ++++ +++
>++++ +++
>++++ ++++ ++
>---
>++++
>---- ---- ---- ----
Zip back to start printing as we go
+[-.<+]-
