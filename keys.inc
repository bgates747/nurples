; 1 Shift
    bit 0,(ix+0)
    jr z,@@
@@:
; 2 Ctrl
    bit 1,(ix+0)
    jr z,@@
@@:
; 3 Alt
    bit 2,(ix+0)
    jr z,@@
@@:
; 4 LeftShift
    bit 3,(ix+0)
    jr z,@@
@@:
; 5 LeftCtrl
    bit 4,(ix+0)
    jr z,@@
@@:
; 6 LeftAlt
    bit 5,(ix+0)
    jr z,@@
@@:
; 7 RightShift
    bit 6,(ix+0)
    jr z,@@
@@:
; 8 RightCtrl
    bit 7,(ix+0)
    jr z,@@
@@:
; 9 RightAlt
    bit 0,(ix+1)
    jr z,@@
@@:
; 10 MouseSelect
    bit 1,(ix+1)
    jr z,@@
@@:
; 11 MouseMenu
    bit 2,(ix+1)
    jr z,@@
@@:
; 12 MouseAdjust
    bit 3,(ix+1)
    jr z,@@


@@:
; 17 Q
    bit 0,(ix+2)
    jr z,@@
@@:
; 18 Num3
    bit 1,(ix+2)
    jr z,@@
@@:
; 19 Num4
    bit 2,(ix+2)
    jr z,@@
@@:
; 20 Num5
    bit 3,(ix+2)
    jr z,@@
@@:
; 21 F4
    bit 4,(ix+2)
    jr z,@@
@@:
; 22 Num8
    bit 5,(ix+2)
    jr z,@@
@@:
; 23 F7
    bit 6,(ix+2)
    jr z,@@
@@:
; 24 Minus
    bit 7,(ix+2)
    jr z,@@
@@:
; 25 Hat
    bit 0,(ix+3)
    jr z,@@
@@:
; 26 Left
    bit 1,(ix+3)
    jr z,@@
@@:
; 27 Kpd6
    bit 2,(ix+3)
    jr z,@@
@@:
; 28 Kpd7
    bit 3,(ix+3)
    jr z,@@
@@:
; 29 F11
    bit 4,(ix+3)
    jr z,@@
@@:
; 30 F12
    bit 5,(ix+3)
    jr z,@@
@@:
; 31 F10
    bit 6,(ix+3)
    jr z,@@
@@:
; 32 ScrollLock
    bit 7,(ix+3)
    jr z,@@
@@:
; 33 F0
    bit 0,(ix+4)
    jr z,@@
@@:
; 34 W
    bit 1,(ix+4)
    jr z,@@
@@:
; 35 E
    bit 2,(ix+4)
    jr z,@@
@@:
; 36 T
    bit 3,(ix+4)
    jr z,@@
@@:
; 37 Num7
    bit 4,(ix+4)
    jr z,@@
@@:
; 38 I
    bit 5,(ix+4)
    jr z,@@
@@:
; 39 Num9
    bit 6,(ix+4)
    jr z,@@
@@:
; 40 Num0
    bit 7,(ix+4)
    jr z,@@
@@:
; 41 Underscore
    bit 0,(ix+5)
    jr z,@@
@@:
; 42 Down
    bit 1,(ix+5)
    jr z,@@
@@:
; 43 Kpd8
    bit 2,(ix+5)
    jr z,@@
@@:
; 44 Kpd9
    bit 3,(ix+5)
    jr z,@@
@@:
; 45 Break
    bit 4,(ix+5)
    jr z,@@
@@:
; 46 Tilde
    bit 5,(ix+5)
    jr z,@@
@@:
; 48 Backspace
    bit 7,(ix+5)
    jr z,@@
@@:
; 49 Num1
    bit 0,(ix+6)
    jr z,@@
@@:
; 50 Num2
    bit 1,(ix+6)
    jr z,@@
@@:
; 51 D
    bit 2,(ix+6)
    jr z,@@
@@:
; 52 R
    bit 3,(ix+6)
    jr z,@@
@@:
; 53 Num6
    bit 4,(ix+6)
    jr z,@@
@@:
; 54 U
    bit 5,(ix+6)
    jr z,@@
@@:
; 55 O
    bit 6,(ix+6)
    jr z,@@
@@:
; 56 P
    bit 7,(ix+6)
    jr z,@@
@@:
; 57 LeftBracket
    bit 0,(ix+7)
    jr z,@@
@@:
; 58 Up
    bit 1,(ix+7)
    jr z,@@
@@:
; 59 KpdPlus
    bit 2,(ix+7)
    jr z,@@
@@:
; 60 KpdMinus
    bit 3,(ix+7)
    jr z,@@
@@:
; 61 KpdEnter
    bit 4,(ix+7)
    jr z,@@
@@:
; 62 Insert
    bit 5,(ix+7)
    jr z,@@
@@:
; 63 Home
    bit 6,(ix+7)
    jr z,@@
@@:
; 64 PgUp
    bit 7,(ix+7)
    jr z,@@
@@:
; 65 Caps
    bit 0,(ix+8)
    jr z,@@
@@:
; 66 A
    bit 1,(ix+8)
    jr z,@@
@@:
; 67 X
    bit 2,(ix+8)
    jr z,@@
@@:
; 68 F
    bit 3,(ix+8)
    jr z,@@
@@:
; 69 Y
    bit 4,(ix+8)
    jr z,@@
@@:
; 70 J
    bit 5,(ix+8)
    jr z,@@
@@:
; 71 K
    bit 6,(ix+8)
    jr z,@@
@@:
; 72 At
    bit 7,(ix+8)
    jr z,@@
@@:
; 73 Colon
    bit 0,(ix+9)
    jr z,@@
@@:
; 74 Enter
    bit 1,(ix+9)
    jr z,@@
@@:
; 75 KpdFwdSlash
    bit 2,(ix+9)
    jr z,@@
@@:
; 76 KpdDel
    bit 3,(ix+9)
    jr z,@@
@@:
; 77 KpdDot
    bit 4,(ix+9)
    jr z,@@
@@:
; 78 NumLock
    bit 5,(ix+9)
    jr z,@@
@@:
; 79 PgDn
    bit 6,(ix+9)
    jr z,@@
@@:
; 81 ShiftLock
    bit 0,(ix+10)
    jr z,@@
@@:
; 82 S
    bit 1,(ix+10)
    jr z,@@
@@:
; 83 C
    bit 2,(ix+10)
    jr z,@@
@@:
; 84 G
    bit 3,(ix+10)
    jr z,@@
@@:
; 85 H
    bit 4,(ix+10)
    jr z,@@
@@:
; 86 N
    bit 5,(ix+10)
    jr z,@@
@@:
; 87 L
    bit 6,(ix+10)
    jr z,@@
@@:
; 88 Semicolon
    bit 7,(ix+10)
    jr z,@@
@@:
; 89 RightBracket
    bit 0,(ix+11)
    jr z,@@
@@:
; 90 Delete
    bit 1,(ix+11)
    jr z,@@
@@:
; 92 KpdStar
    bit 3,(ix+11)
    jr z,@@
@@:
; 93 KpdComma
    bit 4,(ix+11)
    jr z,@@
@@:
; 94 KpdPlus
    bit 5,(ix+11)
    jr z,@@
@@:
; 96 Underscore1
    bit 7,(ix+11)
    jr z,@@
@@:
; 97 Tab
    bit 0,(ix+12)
    jr z,@@
@@:
; 98 Z
    bit 1,(ix+12)
    jr z,@@
@@:
; 99 Space
    bit 2,(ix+12)
    jr z,@@
@@:
; 100 V
    bit 3,(ix+12)
    jr z,@@
@@:
; 101 B
    bit 4,(ix+12)
    jr z,@@
@@:
; 102 M
    bit 5,(ix+12)
    jr z,@@
@@:
; 103 Comma
    bit 6,(ix+12)
    jr z,@@
@@:
; 104 Dot
    bit 7,(ix+12)
    jr z,@@
@@:
; 105 ForwardSlash
    bit 0,(ix+13)
    jr z,@@
@@:
; 106 CopyEnd
    bit 1,(ix+13)
    jr z,@@
@@:
; 107 Kpd0
    bit 2,(ix+13)
    jr z,@@
@@:
; 108 Kpd1
    bit 3,(ix+13)
    jr z,@@
@@:
; 109 Kpd3
    bit 4,(ix+13)
    jr z,@@
@@:
; 113 Escape
    bit 0,(ix+14)
    jr z,@@
@@:
; 114 F1
    bit 1,(ix+14)
    jr z,@@
@@:
; 115 F2
    bit 2,(ix+14)
    jr z,@@
@@:
; 116 F3
    bit 3,(ix+14)
    jr z,@@
@@:
; 117 F5
    bit 4,(ix+14)
    jr z,@@
@@:
; 118 F6
    bit 5,(ix+14)
    jr z,@@
@@:
; 119 F8
    bit 6,(ix+14)
    jr z,@@
@@:
; 120 F9
    bit 7,(ix+14)
    jr z,@@
@@:
; 122 Right
    bit 1,(ix+15)
    jr z,@@
@@:
; 123 Kpd4
    bit 2,(ix+15)
    jr z,@@
@@:
; 124 Kpd5
    bit 3,(ix+15)
    jr z,@@
@@:
; 125 Kpd2
    bit 4,(ix+15)
    jr z,@@
@@:
; 126 WinLeft
    bit 5,(ix+15)
    jr z,@@
@@:
; 127 WinRight
    bit 6,(ix+15)
    jr z,@@
@@:
; 128 WinMenu22223
    bit 7,(ix+15)
    jr z,@@
@@:
