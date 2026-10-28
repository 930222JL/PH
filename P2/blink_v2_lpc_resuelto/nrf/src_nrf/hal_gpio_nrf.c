/* *****************************************************************************
 * P.H.2025: TODO
 */
 
 #include "hal_gpio.h"
 #include "nrf.h"
 
 #define GPIO_PUERTO(gpio) (((gpio) >> 5) ? NRF_P1:NRF_P0) //compruebo el puerto (nº de pin en los 5 bits bajos, puerto en los siguientes)
 #define GIPO_BIT(gpio) ((gpio) & 0x1Fu)										//cojo el bit(bits bajos)
 
void hal_gpio_iniciar(void){
 for(uint32_t i=0;i<32;i++) NRF_P0->PIN_CNF[i]=0;				//inicializo a 0 los 32 pines del puerto 0
 for(uint32_t i=0;i<16;i++) NRF_P1->PIN_CNF[i]=0;				//inicializo a 0 los 16 pines del puerto 1
}
 
void hal_gpio_sentido(HAL_GPIO_PIN_T gpio, hal_gpio_pin_dir_t direccion){
 uint32_t masc = (1UL << GIPO_BIT(gpio));								//mascara con todo 0 menos en la posicion del pin
 if(direccion==HAL_GPIO_PIN_DIR_INPUT) GPIO_PUERTO(gpio)->DIRCLR=masc;		//le metemos la mascara a DIRCLR si es entrada
	else if(direccion==HAL_GPIO_PIN_DIR_OUTPUT) GPIO_PUERTO(gpio)->DIRSET=masc;	//le metemos la mascar a DIRSET si es salida
}
 
uint32_t hal_gpio_leer(HAL_GPIO_PIN_T gpio){
	uint32_t masc = (1UL << GIPO_BIT(gpio));
	return ((GPIO_PUERTO(gpio)->IN & masc) !=0);						//con el and si hay un 1 en el puerto del gpio devuelve masc que es !=0 por lo que devuelve 1
}

void hal_gpio_escribir(HAL_GPIO_PIN_T gpio, uint32_t valor){
	uint32_t masc = (1UL << GIPO_BIT(gpio));
	if((valor & 0x01) == 0) GPIO_PUERTO(gpio)->OUTCLR=masc;	//1 en poner a 0
	else GPIO_PUERTO(gpio)->OUTSET=masc;				// 1 en poner a 1
}
