VERSION		EQU	37
REVISION	EQU	73
DATE	MACRO
		dc.b	'19.4.98'
	ENDM
VERS	MACRO
		dc.b	'SegTracker 37.73'
	ENDM
VSTRING	MACRO
		dc.b	'SegTracker 37.73 (19.4.98)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: SegTracker 37.73 (19.4.98)',0
	ENDM
