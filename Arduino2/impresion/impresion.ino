/*
Usa un bucle FOR para los datos e imprime un número en varios formatos.
*/
int x = 0;    // variable

void setup() {
  Serial.begin(9600);      // abre el puerto serie a 9600 bps:    
}

void loop() {  
  // print labels 
  Serial.print("SIN FORMATO");       // imprime un texto
  Serial.print("\t");              // imprime un tabulado

  Serial.print("DEC");  
  Serial.print("\t");      

  Serial.print("HEX"); 
  Serial.print("\t");   

  Serial.print("OCT");
  Serial.print("\t");

  Serial.print("BIN");
  Serial.print("\t"); 

  Serial.println("BYTE");

  for(x=0; x< 64; x++){    // solo una parte de la tabla

    // imprime en varios formatos:
    Serial.print(x);       // imprime como codificado ASCII decimal - igual que "DEC"
    Serial.print("\t");    // imprime un tabulado

    Serial.print(x, DEC);  // imprime como codificado ASCII decimal
    Serial.print("\t");    // imprime un tabulado

    Serial.print(x, HEX);  // imprime como codificado ASCII hexadecimal
    Serial.print("\t");    // imprime un tabulado

    Serial.print(x, OCT);  // imprime como codificado ASCII octal
    Serial.print("\t");    // imprime un tabulado

    Serial.print(x, BIN);  // imprime como codificado ASCII binario
    Serial.print("\t");    // imprime un tabulado

    Serial.println(x, BYTE);    // imprime el valor en bruto del byte, 
    //                             y añade el salto de linea con "println"
    delay(200);            // espera 200 milisegundos
  }
  Serial.println("");      //imprime otro salto de linea
}
