   DOSSEG
   .MODEL small
   .CODE

   PUBLIC _blit_8514
_blit_8514 PROC
   ARG x0:WORD, y0:WORD, x1:WORD, y1:WORD, col:WORD
   push bp
   mov bp, sp

   mov ax, col   ; foreground colour
   mov dx, 0a6e8h
   out dx, ax

   mov ax, 023h   ; background color
   mov dx, 0a2e8h
   out dx, ax

   mov ax, 27h    ; colour src = blit buffer, src
   mov dx, 0bae8h ; foreground mix
   out dx, ax

   mov ax, 03h    ; colour src = dest
   mov dx, 0b6e8h ; background mix
   out dx, ax

   mov ax, 1010000011000000b
   mov dx, 0bee8h ; pixel control
   out dx, ax

   mov ax, col
   shl ax, 1
   mov dx, 0aee8h ; read mask
   out dx, ax

   mov ax, 0ffffh ; write mask
   mov dx, 0aae8h
   out dx, ax

   mov dx, 86e8h ; current x
   mov ax, x0
   mov bx, ax
   out dx, ax

   mov dx, 82e8h ; current y
   mov ax, y0
   mov cx, ax
   out dx, ax

   mov ax, x1    ; rect width
   sub ax, bx
   mov dx, 96e8h
   out dx, ax

   mov ax, y1    ; rect height
   sub ax, cx
   mov dx, 0bee8h
   out dx, ax

   mov ax, bx    ; destination x
   sub ax, 640
   mov dx, 8ee8h
   out dx, ax

   mov ax, cx    ; destination y
   mov dx, 8ae8h
   out dx, ax

   mov dx, 9ae8h  ; check status
wait_ge_blit:
   in  ax, dx
   test ah, 2
   jnz wait_ge_blit

   mov dx, 9ae8h
   mov ax, 1100000010110001b ; draw command = blit
   out dx, ax  ; blit

   mov dx, 9ae8h
wait_ge_blit2:
   in  ax, dx
   test ah, 2
   jnz wait_ge_blit2

   mov dx, 0bee8h ; pixel control
   mov ax, 1010000000000000b
   out dx, ax
 
   pop bp
   ret
_blit_8514 ENDP

   PUBLIC _xor_8514
_xor_8514 PROC
   mov dx, 0bae8h
   mov ax, 025h
   out dx, ax
   ret
_xor_8514 ENDP

   PUBLIC _noxor_8514
_noxor_8514 PROC
   mov dx, 0bae8h
   mov ax, 027h
   out dx, ax
   ret
_noxor_8514 ENDP

   PUBLIC _set_scissor_8514
_set_scissor_8514 PROC
   ARG x0:WORD, y0:WORD, x1:WORD, y1:WORD
   push bp
   mov bp, sp

   mov dx, 0bee8h
   mov ax, x0
   or ah, 020h ; function 2 left scissor
   out dx, ax

   mov ax, y0
   or ah, 010h ; function 1 top scissor
   out dx, ax

   mov ax, x1
   or ah, 040h ; function 1 right scissor
   out dx, ax

   mov ax, y1
   or ah, 030h ; function 3 bottom scissor
   out dx, ax
   
   pop bp
   ret
_set_scissor_8514 ENDP

   PUBLIC _set_fgcolour_8514
_set_fgcolour_8514 PROC
   ARG colour:WORD
   push bp
   mov bp, sp

   mov ax, colour
   mov dx, 0a6e8h  ; foreground colour
   out dx, ax

   pop bp
   ret
_set_fgcolour_8514 ENDP

   PUBLIC _set_current_8514
_set_current_8514 PROC
   ARG x0:WORD, y0:WORD
   push bp
   mov bp, sp

   mov dx, 86e8h ; current x
   mov ax, x0
   out dx, ax

   mov dx, 82e8h ; current y
   mov ax, y0
   out dx, ax

   pop bp
   ret
_set_current_8514 ENDP

   PUBLIC _line_8514
_line_8514 PROC
   ARG x0:WORD, y0:WORD, x1:WORD, y1:WORD
   push bp
   mov bp, sp
   push si

   mov si, 1010001000010111b ; write + nibble mode + no last pixel
                           ;       + coord mode + draw + draw poly boundary
   
   mov ax, y1
   cmp ax, y0
   jae no_reverse
   mov ax, x0
   xchg ax, x1
   mov x0, ax
   mov ax, y0
   xchg ax, y1
   mov y0, ax

no_reverse:
   mov dx, 86e8h ; current x
   mov ax, x0
   out dx, ax

   mov dx, 82e8h ; current y
   mov ax, y0
   out dx, ax

   mov bx, x1    ; dx
   sub bx, x0
   jae startx_lt_endx
   neg bx        ; |dx|
 
   mov cx, y0    ; dy
   sub cx, y1
   jae starty_gte_endy1
   neg cx
   or  si, 128   ; positive y direction
starty_gte_endy1:

   cmp cx, bx
   jb abs_dy_lt_abs_dx1
   xchg bx, cx
   or  si, 64    ; y major
abs_dy_lt_abs_dx1:   

   mov ax, cx    ; error term
   shl ax, 1
   sub ax, bx
   mov dx, 92e8h
   out dx, ax

   mov ax, bx
   mov dx, 96e8h  ; major axis pixel count
   out dx, ax

   mov ax, cx
   shl ax, 1
   mov dx, 8ae8h  ; axial step register
   out dx, ax
   shl bx, 1
   sub ax, bx
   mov dx, 8ee8h  ; diagonal step register
   out dx, ax

   mov dx, 9ae8h  ; check status
wait_ge1:
   in  ax, dx
   test ah, 2
   jnz wait_ge1

   mov ax, si     ; draw
   out dx, ax

   pop si
   pop bp
   ret

startx_lt_endx:
   or  si, 32    ; positive x direction

   mov cx, y0    ; dy
   sub cx, y1
   jae starty_gte_endy2
   neg cx
   or  si, 128   ; positive y direction
starty_gte_endy2:

   cmp cx, bx
   jb abs_dy_lt_abs_dx2
   xchg bx, cx
   or  si, 64
abs_dy_lt_abs_dx2:   

   mov ax, cx    ; error term
   shl ax, 1
   sub ax, bx
   dec ax
   mov dx, 92e8h
   out dx, ax

   mov ax, bx
   mov dx, 96e8h  ; major axis pixel count
   out dx, ax

   mov ax, cx
   shl ax, 1
   mov dx, 8ae8h  ; axial step register
   out dx, ax
   shl bx, 1
   sub ax, bx
   mov dx, 8ee8h  ; diagonal step register
   out dx, ax

   mov dx, 9ae8h  ; check status
wait_ge2:
   in  ax, dx
   test ah, 2
   jnz wait_ge2

   mov ax, si     ; draw
   out dx, ax
   
   pop si
   pop bp
   ret
_line_8514 ENDP

   PUBLIC _fill_8514
_fill_8514 PROC
   ARG x0:WORD, y0:WORD, x1:WORD, y1:WORD, colour:WORD
   push bp
   mov bp, sp

   mov ax, colour
   mov dx, 0aae8h ; write mask
   out dx, ax
   mov ax, 0ffffh
   mov dx, 8ae8h ; read mask
   out dx, ax

   mov ax, 1010000000000110b
   mov dx, 0bee8h ; pixel control register
   out dx, ax
 
   mov ax, x0
   mov dx, 86e8h  ; current x
   out dx, ax

   sub ax, x1
   neg ax
   mov dx, 96e8h  ; rect width
   out dx, ax

   mov ax, y0
   mov dx, 82e8h  ; current y
   out dx, ax

   sub ax, y1
   neg ax
   mov dx, 0bee8h ; rect height
   out dx, ax

   mov dx, 9ae8h  ; check status
wait_ge:
   in  ax, dx
   test ah, 2
   jnz wait_ge

   mov ax, 100000010110011b ; fill rect hor
   out dx, ax
  
   mov ax, 1010000000000000b
   mov dx, 0bee8h ; pixel control register
   out dx, ax

   pop bp
   ret
_fill_8514 ENDP

   PUBLIC _blank_8514
_blank_8514 PROC
   ARG x0:WORD, y0:WORD, x1:WORD, y1:WORD
   push bp
   mov bp, sp

   xor ax, ax
   mov dx, 0a6e8h  ; foreground colour
   out dx, ax

   mov ax, 0ffffh
   mov dx, 00aae8h ; write mask
   out dx, ax
 
   mov ax, x1
   mov dx, 86e8h  ; current x
   out dx, ax

   sub ax, x0
   mov dx, 96e8h  ; rect width
   out dx, ax

   mov ax, y1
   mov dx, 82e8h  ; current y
   out dx, ax

   sub ax, y0
   mov dx, 0bee8h ; rect height
   out dx, ax

   mov dx, 9ae8h  ; check status
wait_ge_blank:
   in  ax, dx
   test ah, 2
   jnz wait_ge_blank

   mov ax, 100000000010011b ; fill rect hor
   out dx, ax
  
   pop bp
   ret
_blank_8514 ENDP
   END