// Proyecto Hardware 2026 — Capa neuronal fully-connected en Q12 con AAPCS
// Referencia C + verificación + main de prueba.
// ARM7TDMI (ARMv4T) / Keil µVision.
// v0.9 - Enrique Torres && chatGPT

// ======================================================================
// INCLUDES Y DEFINICIONES
// ======================================================================
#include "dense_q12.h"
#include <stdint.h>

typedef struct{
	const int16_t *input, *weights, *bias, *esperado;
} banco_pruebas;

// ======================================================================
// REFERENCIA EN C
// ======================================================================

/* ============================================================
   Neurona fully-connected en Q12
   ============================================================ */

int16_t neuron_q12_C(
    const int16_t *input,
    const int16_t *weights,
    uint16_t n,
    int16_t bias_q12,
    int16_t clamp_min,
    int16_t clamp_max) {

    // El bias está en Q12; el acumulador trabaja en Q24.
    // Por tanto, bias_q12 se desplaza 12 bits a la izquierda.
    int32_t acc = ((int32_t)bias_q12) << Q_SHIFT;

    for (uint16_t i = 0; i < n; ++i) {
        // input[i] y weights[i] están en Q12.
        // Su producto queda en Q24 y se acumula directamente.
        acc += (int32_t)input[i] * (int32_t)weights[i];
    }

    // Volvemos de Q24 a Q12.
    // En ARM esto corresponde naturalmente a un desplazamiento aritmético ASR #12.
    int32_t result_q12 = acc >> Q_SHIFT;

    // Activación/saturación final.
    return clamp_i16_q12(result_q12, clamp_min, clamp_max);
}

/* ============================================================
   Capa fully-connected completa en Q12
   ============================================================ */

uint32_t dense_layer_q12_C(
    const int16_t *input,
    const int16_t *weights,
    const int16_t *bias,
    int16_t *output,
    uint16_t input_size,
    uint16_t output_size,
    int16_t clamp_min,
    int16_t clamp_max) {

    uint32_t checksum = 0;

    for (uint16_t o = 0; o < output_size; ++o) {
        const int16_t *weights_o = &weights[o * input_size];

        int16_t y = neuron_q12_C(
            input,
            weights_o,
            input_size,
            bias[o],
            clamp_min,
            clamp_max
        );

        output[o] = y;
        checksum = checksum * 33u + (uint16_t)y;
    }

    return checksum;
}

/* ============================================================
   Capa C llamando a neurona ARM/Thumb
   ============================================================ */

uint32_t dense_layer_q12_C_ARM(
    const int16_t *input,
    const int16_t *weights,
    const int16_t *bias,
    int16_t *output,
    uint16_t input_size,
    uint16_t output_size,
    int16_t clamp_min,
    int16_t clamp_max) {

    uint32_t checksum = 0;

    for (uint16_t o = 0; o < output_size; ++o) {
        const int16_t *weights_o = &weights[o * input_size];

        int16_t y = neuron_q12_ARM(
            input,
            weights_o,
            input_size,
            bias[o],
            clamp_min,
            clamp_max
        );

        output[o] = y;
        checksum = checksum * 33u + (uint16_t)y;
    }

    return checksum;
}

uint32_t dense_layer_q12_C_THB(
    const int16_t *input,
    const int16_t *weights,
    const int16_t *bias,
    int16_t *output,
    uint16_t input_size,
    uint16_t output_size,
    int16_t clamp_min,
    int16_t clamp_max) {

    uint32_t checksum = 0;

    for (uint16_t o = 0; o < output_size; ++o) {
        const int16_t *weights_o = &weights[o * input_size];

        int16_t y = neuron_q12_THB(
            input,
            weights_o,
            input_size,
            bias[o],
            clamp_min,
            clamp_max
        );

        output[o] = y;
        checksum = checksum * 33u + (uint16_t)y;
    }

    return checksum;
}

// ======================================================================
// VERIFICACIÓN AUTOMÁTICA
// ======================================================================

/**
 * @brief   Ejecuta C, C_ARM, C_THB, ARM_C, ARM y THB y verifica resultados.
 *
 * @param   input        Vector de entrada Q12.
 * @param   weights      Matriz de pesos Q12.
 * @param   bias         Vector de bias Q12.
 * @param   resultado    Se deja aquí la salida de la versión C.
 * @param   input_size   Número de entradas.
 * @param   output_size  Número de salidas/neuronas.
 * @param   clamp_min    Límite inferior Q12.
 * @param   clamp_max    Límite superior Q12.
 * @return  1 si todos devuelven el mismo checksum y el mismo vector; 0 si difieren.
 */
uint8_t dense_q12_verificar(
    const int16_t *input,
    const int16_t *weights,
    const int16_t *bias,
    int16_t *resultado,
    uint16_t input_size,
    uint16_t output_size,
    int16_t clamp_min,
    int16_t clamp_max) {

    static int16_t out_C     [OUTPUT_SIZE];
    static int16_t out_C_ARM [OUTPUT_SIZE];
    static int16_t out_C_THB [OUTPUT_SIZE];
    static int16_t out_ARM_C [OUTPUT_SIZE];
    static int16_t out_ARM   [OUTPUT_SIZE];
    static int16_t out_THB   [OUTPUT_SIZE];

    uint32_t chk_C = dense_layer_q12_C(
        input, weights, bias, out_C,
        input_size, output_size, clamp_min, clamp_max);

    uint32_t chk_C_ARM = dense_layer_q12_C_ARM(
        input, weights, bias, out_C_ARM,
        input_size, output_size, clamp_min, clamp_max);

    uint32_t chk_C_THB = dense_layer_q12_C_THB(
        input, weights, bias, out_C_THB,
        input_size, output_size, clamp_min, clamp_max);

    uint32_t chk_ARM_C = dense_layer_q12_ARM_C(
        input, weights, bias, out_ARM_C,
        input_size, output_size, clamp_min, clamp_max);

    uint32_t chk_ARM = dense_layer_q12_ARM(
        input, weights, bias, out_ARM,
        input_size, output_size, clamp_min, clamp_max);

    uint32_t chk_THB = dense_layer_q12_THB(
        input, weights, bias, out_THB,
        input_size, output_size, clamp_min, clamp_max);

    for (uint16_t i = 0; i < output_size; ++i) {
        resultado[i] = out_C[i];
    }

    if (!(chk_C == chk_C_ARM && chk_C == chk_C_THB &&
          chk_C == chk_ARM_C && chk_C == chk_ARM && chk_C == chk_THB)) {
        return 0;
    }
		if (!vectores_iguales_i16(out_C, out_C_ARM, output_size)) return 0;
    if (!vectores_iguales_i16(out_C, out_C_THB, output_size)) return 0;
    if (!vectores_iguales_i16(out_C, out_ARM_C, output_size)) return 0;
    if (!vectores_iguales_i16(out_C, out_ARM,   output_size)) return 0;
    if (!vectores_iguales_i16(out_C, out_THB,   output_size)) return 0;

    return 1;
}


//banco de pruebas que se encarga de verificar el funcionamiento de las funciones implementadas en esta practica para no solo tener una verificaciónen la que 
//se comparan los resultados con las versiones en c si no que también se verificar los resultados esperados en distintos casos con los devueltos por la diferentes versiones
uint8_t func_verificar(){
	//banco de pruebas para todo 0
	static const int16_t input_zero[INPUT_SIZE] = {0,0,0,0,0,0,0,0};
	static const int16_t weights_zero[OUTPUT_SIZE * INPUT_SIZE] = {0}; 
	static const int16_t bias_zero[OUTPUT_SIZE] = {0,0,0,0,0};
	static const int16_t resultado_zero[OUTPUT_SIZE] = {0,0,0,0,0};
	//banco de pruebas para comprobar que se evitan saturaciones con clamp_max
	static const int16_t input_sat_max[INPUT_SIZE] = {
		Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE
	};
	static const int16_t weights_sat_max[OUTPUT_SIZE * INPUT_SIZE] = {
		Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0
	}; 
	static const int16_t bias_sat_max[OUTPUT_SIZE] = {0,0,0,0,0};
	static const int16_t resultado_sat_max[OUTPUT_SIZE] = {CLAMP_MAX_Q12,Q_ONE/16,Q_ONE/16,Q_ONE/16,Q_ONE/16};
	//banco de pruebas para comprobar que se evitan saturaciones con clamp_min
	static const int16_t input_sat_min[INPUT_SIZE] = {
		Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE, Q_ONE
	};
	static const int16_t weights_sat_min[OUTPUT_SIZE * INPUT_SIZE] = {
		-Q_ONE, -Q_ONE, -Q_ONE, -Q_ONE, -Q_ONE, -Q_ONE, -Q_ONE, -Q_ONE,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0,
		Q_ONE/16,0,0,0,0,0,0,0
	};
	static const int16_t bias_sat_min[OUTPUT_SIZE] = {0,0,0,0,0};
	static const int16_t resultado_sat_min[OUTPUT_SIZE] = {CLAMP_MIN_Q12,Q_ONE/16,Q_ONE/16,Q_ONE/16,Q_ONE/16};
	//banco de pruebas para comprobar que el bias se utiliza adecuadamente
	static const int16_t input_bias[INPUT_SIZE] = {0,0,0,0,0,0,0,0};
	static const int16_t weights_bias[OUTPUT_SIZE * INPUT_SIZE] = {0}; 
	static const int16_t bias_bias[OUTPUT_SIZE] = {0,Q_ONE,-Q_ONE,Q_ONE/4,Q_ONE/16};
	static const int16_t resultado_bias[OUTPUT_SIZE] = {0,CLAMP_MAX_Q12,0,Q_ONE/4,Q_ONE/16};
	//banco de pruebas para comprobar que no hay ningun error de offset
	static const int16_t input_index[INPUT_SIZE] = {Q_ONE/2,Q_ONE/4,Q_ONE/8,Q_ONE/16,Q_ONE/32,Q_ONE/64,Q_ONE/128,Q_ONE/256};
	static const int16_t weights_index[OUTPUT_SIZE * INPUT_SIZE] = {
		Q_ONE,0,0,0,0,0,0,0,
		0,Q_ONE,0,0,0,0,0,0,
		0,0,Q_ONE,0,0,0,0,0,
		0,0,0,Q_ONE,0,0,0,0,
		0,0,0,0,Q_ONE,0,0,0
	};
	static const int16_t bias_index[OUTPUT_SIZE] = {0,0,0,0,0};
	static const int16_t resultado_index[OUTPUT_SIZE] = {Q_ONE/2,Q_ONE/4,Q_ONE/8,Q_ONE/16,Q_ONE/32};
	
	banco_pruebas casos[5] = {
		input_zero, weights_zero, bias_zero, resultado_zero,
		input_sat_max, weights_sat_max, bias_sat_max, resultado_sat_max,
		input_sat_min, weights_sat_min, bias_sat_min, resultado_sat_min,
		input_bias, weights_bias, bias_bias, resultado_bias,
		input_index, weights_index, bias_index, resultado_index
	};
	
	static int16_t resultado[OUTPUT_SIZE] __attribute__((aligned(8)));
	for(uint16_t i=0; i<5;++i){
		dense_layer_q12_ARM_C(casos[i].input,casos[i].weights,casos[i].bias,resultado,INPUT_SIZE,OUTPUT_SIZE,CLAMP_MIN_Q12,CLAMP_MAX_Q12);
		if(!vectores_iguales_i16(casos[i].esperado,resultado,OUTPUT_SIZE)){
			return 0;
		}
		dense_layer_q12_ARM(casos[i].input,casos[i].weights,casos[i].bias,resultado,INPUT_SIZE,OUTPUT_SIZE,CLAMP_MIN_Q12,CLAMP_MAX_Q12);
		if(!vectores_iguales_i16(casos[i].esperado,resultado,OUTPUT_SIZE)){
			return 0;
		}
		dense_layer_q12_THB(casos[i].input,casos[i].weights,casos[i].bias,resultado,INPUT_SIZE,OUTPUT_SIZE,CLAMP_MIN_Q12,CLAMP_MAX_Q12);
		if(!vectores_iguales_i16(casos[i].esperado,resultado,OUTPUT_SIZE)){
			return 0;
		}
	}
	return 1;
}

/* ============================================================
   MAIN de prueba
   ============================================================ */
int main(void) {
    // Vector de entrada de prueba, en Q12.
    // Se puede visualizar en Keil como enteros int16_t.
    // Ejemplos:
    //   Q_ONE      = 4096  ->  1.0
    //   Q_ONE / 2  = 2048  ->  0.5
    //  -Q_ONE / 4  = -1024 -> -0.25
    static const int16_t input[INPUT_SIZE] __attribute__((aligned(8))) = {
         Q_ONE,        //  1.00
         Q_ONE / 2,    //  0.50
        -Q_ONE / 4,    // -0.25
         3 * Q_ONE / 4,//  0.75
         0,            //  0.00
        -Q_ONE / 2,    // -0.50
         Q_ONE / 4,    //  0.25
         Q_ONE / 8     //  0.125
    };

    // Matriz de pesos: OUTPUT_SIZE filas x INPUT_SIZE columnas, en Q12.
    // Cada fila corresponde a una neurona de salida.
    static const int16_t weights[OUTPUT_SIZE * INPUT_SIZE] __attribute__((aligned(8))) = {
        // Neurona 0
         Q_ONE / 2,   Q_ONE / 2,  -Q_ONE / 4,   Q_ONE / 4,
         0,          -Q_ONE / 8,   Q_ONE / 4,   0,

        // Neurona 1
        -Q_ONE / 2,   Q_ONE / 4,   Q_ONE / 2,  -Q_ONE / 4,
         Q_ONE / 8,   Q_ONE / 4,  -Q_ONE / 8,   Q_ONE / 8,

        // Neurona 2
         Q_ONE,      -Q_ONE,       Q_ONE / 2,   0,
         0,           Q_ONE / 4,   Q_ONE / 4,  -Q_ONE / 2,

        // Neurona 3
        -Q_ONE / 4,   Q_ONE / 4,   Q_ONE / 4,   Q_ONE / 4,
        -Q_ONE / 2,   Q_ONE / 2,   0,           Q_ONE / 8,

        // Neurona 4
         Q_ONE / 8,  -Q_ONE / 8,   Q_ONE / 8,  -Q_ONE / 8,
         Q_ONE / 2,   Q_ONE / 2,   Q_ONE / 4,   Q_ONE / 4
    };

    // Un bias por neurona, también en Q12 e int16_t.
    static const int16_t bias[OUTPUT_SIZE] __attribute__((aligned(8))) = {
         0,
         Q_ONE / 8,
        -Q_ONE / 4,
         Q_ONE / 4,
         0
    };

    // Búfer para quedarse con la salida de referencia C.
    static int16_t resultado[OUTPUT_SIZE] __attribute__((aligned(8)));

    // Verificación.
    uint8_t ok = dense_q12_verificar(
        input,
        weights,
        bias,
        resultado,
        INPUT_SIZE,
        OUTPUT_SIZE,
        CLAMP_MIN_Q12,
        CLAMP_MAX_Q12
    );
		
		ok=func_verificar();

    // Punto de parada para depuración: inspeccionar 'ok' y 'resultado'.
    // En este entorno no hay SO; nos quedamos en bucle.
    (void)ok;
    while (1) { /* no retornar */ }
}
