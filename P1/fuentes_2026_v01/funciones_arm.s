	AREA codigo, CODE, READONLY
	EXPORT dense_layer_q12_ARM_C
	EXPORT neuron_q12_ARM
	EXPORT dense_layer_q12_ARM
	IMPORT neuron_q12_C
	PRESERVE8 {TRUE}
		
	
; función que llama a la función en c que se encarga de calcular cada neurona
dense_layer_q12_ARM_C
	push {lr,fp}
	mov fp,sp
	push{r4-r11}
	mov r4,r3
	ldr r5,[fp,#12]
	ldr r6,[fp,#16]
	ldr r7,[fp,#20]
	push{r7,r6}
	mov r6,r0
	mov r7,r2
	ldr r2,[fp,#8]
	mov r8,#0
	mov r9,#0
	mov r10,r1
	
	; r0=input, r1=weights_o, r2=input_size, r4=output, r5=output_size, r6=input, r7=bias, r8=checksum, r9=o, r10=weights, sp+0=clamp_min, sp+4=clamp_max
	
bucle
	cmp r9,r5
	bge fin_bucle
	ldr r2,[fp,#8]		;r2=input_size
	mov r0,r6			;r0=input
	lsl r9,#1			;o*2 para calcular el offset en bytes
	mul r1,r9,r2		;r1=o*input_size
	add r1,r1,r10		;weights_o=weights[o*input_size]
	ldrsh r3,[r7,r9]	;r3=bias[o]
	bl neuron_q12_C
	strh r0,[r4,r9]		;output[o]=neuron_q12_C
	mov r1,#33			;r1=33
	mul r8,r1,r8		;33u*output[o]
	lsl r0,r0,#16		;cast de output[o]
	lsr r0,r0,#16
	add r8,r8,r0		;checksum+=y*33u
	add r9,r9,#2		;o++	
	lsr r9,#1			;deshago el cambio de offset a bytes para la comparación del bucle
	b bucle
	
fin_bucle
	add sp,sp,#8	;desapilo clamp_min y clamp_max de las llamadas a neuron_q12_C
	mov r0,r8		;return checsum
	pop{r4-r11}
	pop{fp,pc}
	
	
; función neuron_q12_C traducida y optimizada a ARM para su uso por dense_layer_q12_ARM que devuelve una neurona 
; r0=input, r1=weights, r2=input_size, r3=bias, r4=acc, r5=clamp_min, r6=ipnut[i]/clamp_max, r7=weights[i], r8=input[i]*weights[i]
neuron_q12_ARM
	mov ip,sp
	stmdb sp!, {r4-r10,fp,ip,lr,pc}
	sub fp,ip,#4
	sub sp, sp,#16
	lsl r4,r3,#12		;r4=acc
	cmp r2,#0
	beq fin_bucle_neuron
	
bucle_neuron 
	ldrsh r6,[r0],#2	;r6=input[i]
	ldrsh r7,[r1],#2	;r7=weights[i]
	mul r8,r7,r6		
	add r4,r4,r8		;acc+= input[i]*weights[i]
	subs r2,r2,#1
	bne bucle_neuron
	
fin_bucle_neuron
	asr r4,#12		;acc a q12
	ldr r5,[fp,#4]	;r5=clamp_min
	ldr r6,[fp,#8]	;r6clamp_max
	mov r0,r4		
	cmp r4,r5		;clamp_il_q12
	movlt r0,r5
	cmp r4,r6
	movgt r0,r6
	lsl r0,#16
	asr r0,#16		;return uint16_t
	ldmdb fp, {r4-r10,fp,sp,pc}
	
; función dense_layer_q12_c traducida a ARM que se encarga de calcular una capa neuronal completa y que llama a neuron_q12_ARM para que calcule una neurona
dense_layer_q12_ARM
	mov ip,sp
	stmdb sp!, {r4-r10,fp,ip,lr,pc}
	sub fp,ip,#4
	sub sp, sp,#16
	mov r4,r3		;r4=output
	ldr r5,[fp,#8]	;r5=output_size
	ldr r6,[fp,#12]	;r6=clamp_min
	ldr r7,[fp,#16]	;clamp_max
	push{r7,r6}		;clamp_min=sp+0, clamp_max=sp+4
	mov r6,r0		;r6=input
	mov r7,r2		;r7=bias
	ldr r2,[fp,#4]	;r2=input_size
	mov r8,#0
	mov r9,r5
	mov r10,r1
	
	; r0=input, r1=weights_o, r2=input_size, r4=output, r5=output_size, r6=input, r7=bias, r8=checksum, r9=o, r10=weights, sp+0=clamp_min, sp+4=clamp_max
	
	cmp r9,#0
	beq fin_bucle_dense_ARM
bucle_dense_ARM
	mov r0,r6			;r0=input
	ldrsh r3,[r7],#2	;r3=bias[o]
	mov r1,r10
	bl neuron_q12_ARM
	strh r0,[r4],#2		;output[o]=y
	ldr r2,[fp,#4]
	add r10,r10,r2,lsl#1		;r10=weights_o
	mov r1,#33
	mul r8,r1,r8		;checksum*33
	lsl r0,r0,#16
	lsr r0,r0,#16		;(uint16_t)y
	add r8,r8,r0		;r8=checksum*33u + y
	subs r9,r9,#1		;o++
	bne bucle_dense_ARM
	
fin_bucle_dense_ARM
	add sp,sp,#8		;desapilo clamp_min y clamp_max de las llamadas a neuron_q12_C
	mov r0,r8			;return checksum
	ldmdb fp, {r4-r10,fp,sp,pc}
	
	end