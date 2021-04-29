	.686P
	.XMM
	.model	flat,c
	
    .code
_allshr PROC NEAR

;
; Handle shifts of 64 bits or more (if shifting 64 bits or more, the result
; depends only on the high order bit of edx).
;
        cmp     cl,64
        jae     short RETSIGN

;
; Handle shifts of between 0 and 31 bits
;
        cmp     cl, 32
        jae     short MORE32
        shrd    eax,edx,cl
        sar     edx,cl
        ret

;
; Handle shifts of between 32 and 63 bits
;
MORE32:
        mov     eax,edx
        sar     edx,31
        and     cl,31
        sar     eax,cl
        ret

;
; Return double precision 0 or -1, depending on the sign of edx
;
RETSIGN:
        sar     edx,31
        mov     eax,edx
        ret

_allshr ENDP

end
