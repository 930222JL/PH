	AREA codigo, CODE, READONLY
	THUMB
	EXPORT neuron_q12_THB
	EXPORT dense_layer_q12_THB
	IMPORT neuron_q12_C
	PRESERVE8 {TRUE}

; función neuron_q12_C traducida a Thumb para su uso por dense_layer_q12_THB que devuelve una neurona 
neuron_q12_THB
	push {r4-r7}
	lsls r4,r3,#12		; r4 = acc = bias << 12
	movs r5,#0				; idx = 0
	cmp r2,#0
	beq fin_bucle_neuron_THB

bucle_neuron_THB
	ldrsh r6,[r0,r5]		; r6 = input[i]    
	ldrsh r7,[r1,r5]		; r7 = weights[i]
	muls r7,r6,r7			; r7 = r6*r7  
	adds r4,r4,r7			; acc += input[i]*weights[i]
	adds r5,r5,#2			; i += 2 bytes
	subs r2,r2,#1
	bne bucle_neuron_THB

fin_bucle_neuron_THB
	asrs r4,r4,#12		; acc a q12
	ldr r5,[sp,#16]		; r5 = clamp_min
	ldr r6,[sp,#20]		; r6 = clamp_max
	movs r0,r4
	cmp r4,r5				; if (acc < clamp_min) r0 = clamp_min
	bge no_min_THB
	movs r0,r5
no_min_THB
	cmp r4,r6				; if (acc > clamp_max) r0 = clamp_max
	ble no_max_THB
	movs r0,r6
no_max_THB
	lsls r0,r0,#16		; extension de signo de 16 bits
	asrs r0,r0,#16
	pop {r4-r7}
	bx lr

; función dense_layer_q12_c traducida a Thumb que se encarga de calcular una capa neuronal completa 
; y que llama a neuron_q12_THB para que calcule una neurona
dense_layer_q12_THB
	push {r4-r7, lr}
	sub sp,sp,#20
	ldr r4,[sp,#48]		; clamp_min
	str r4,[sp,#0]
	ldr r4,[sp,#52]		; clamp_max
	str r4,[sp,#4]
	str r0,[sp,#8]		; input
	ldr r4,[sp,#44]		; output_size
	str r4,[sp,#12]			; o = output_size
	ldr r4,[sp,#40]			; input_size
	str r4,[sp,#16]
	movs r6,r1					; weights
	movs r5,r2					; bias
	movs r4,r3					; output
	movs r7,#0					; checksum = 0
	ldr r0,[sp,#12]
	cmp r0,#0
	beq fin_bucle_dense_THB

bucle_dense_THB
	ldr r0,[sp,#8]	; r0 = input
	movs r1,r6			; r1 = weights_o
	ldr r2,[sp,#16]	; r2 = input_size (por la llamada a neuron)
	movs r3,#0
	ldrsh r3,[r5,r3]	; r3 = bias[o]
	adds r5,r5,#2		; bias++
	bl neuron_q12_THB	; r0 = y
	strh r0,[r4,#0]	; output[o] = y
	adds r4,r4,#2
	ldr r2,[sp,#16]
	lsls r2,r2,#1
	adds r6,r6,r2		; weights_o += input_size*2 bytes
	movs r1,#33
	muls r7,r1,r7		; checksum*33
	lsls r0,r0,#16
	lsrs r0,r0,#16	; (uint16_t)y
	adds r7,r7,r0		; checksum = checksum*33 + y
	ldr r0,[sp,#12]
	subs r0,r0,#1		; o--
	str r0,[sp,#12]
	bne bucle_dense_THB		

fin_bucle_dense_THB
	movs r0,r7			; return checksum
	add sp,sp,#20	
	pop {r4-r7}
	pop {r1}			
	bx r1				

	END