
typedef struct
{
    int humedad_amb; 
    float temperatura_amb; 
    int humedad_suelo; 
    int intensidad_luz; 
    char ultimo_riego[64];

}sensor_data;

/*Estructura para guardar configuración*/
typedef struct
{
    char UUID[18];      // debe almacenar el identificador recibido en custom message
    char MAC[13];        // almacena la mac del esp
    int hum_sup;        // limite superior de humedad
    int hum_inf;        // Limite inferior de humedad                
    int control_riego;   // Controla el riego automatico

}config_data;

void recibe_confg_hum(char str[], config_data *cfg);
void ultimo_riego(void);
void bytesToHex(unsigned char* bytes, int size, char* hexString); 


/*
UUID (colección)
    MAC (colección)
        - configurations (colección)
            - {esp32_id} (documento)
                - UUID: "{UUID}"
                - MAC: "{MAC}"
                - hum_sup: {hum_sup}
                - hum_inf: {hum_inf}
                - control_riego: {control_riego}

        - sensor_data (colección)
            - {medicion_1} (documento)
                - humedad_amb: {humedad_amb}
                - temperatura_amb: {temperatura_amb}
                - humedad_suelo: {humedad_suelo}
                - intensidad_luz: {intensidad_luz}
                - ultimo_riego: "{ultimo_riego}"
            - {medicion_2} (documento)
                - humedad_amb: {humedad_amb}
                - temperatura_amb: {temperatura_amb}
                - humedad_suelo: {humedad_suelo}
                - intensidad_luz: {intensidad_luz}
                - ultimo_riego: "{ultimo_riego}
            - {medicion_3} (documento)
                - humedad_amb: {humedad_amb}
                - temperatura_amb: {temperatura_amb}
                - humedad_suelo: {humedad_suelo}
                - intensidad_luz: {intensidad_luz}
                - ultimo_riego: "{ultimo_riego}




*/