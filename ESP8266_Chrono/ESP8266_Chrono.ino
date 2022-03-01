// Cronómetro para Velocistas
// El cronómetro se activa con un LDR al que le incide un láser, cuando el laser se corta medimos millis
// Los valores de los tiempos de cada vuelta se meten en las variables Trans1, Trans2 y Trans3
// Hay una tira de leds de Adafruit que indica con luz verde o rojo, las vueltas.

#include <ESP8266WebServer.h>  // Libreía del WebServer

#include <Ticker.h> // Librería para timers
#include <Arduino.h>
Ticker timer;  // creo el objeto timer para el temporizador

//---------------------------------------------------------------
ESP8266WebServer server(80);  // puerto del Servidor

//-------------------VARIABLES GLOBALES--------------------------

int contconexion = 0;

// Parámetros para conexión
const char *ssid = "MIWIFI";
const char *password = "MIPASSWORD";


String XML, XML1, XML2, XML3, xmlTemperatura1, xmlTemperatura2, xmlTemperatura3; //aquí añado los strings xmlTemperatura

unsigned long previousMillis = 0;

#define Laser 13  //defino el laser en la salida digital 2

//Variables del crónometro

int estado = 0;  // variable estado que nos si está en 0 apaga el led y si está en 1 lo enciende
unsigned long Inicio;  // Variable que captura el tiempo en la pulsación de salida
unsigned long Tiempo1;   //Variable que captura el tiempo en la primera pulsación
unsigned long Tiempo2;
unsigned long Tiempo3;
unsigned long Trans1 = 0; // variable que captura el tiempo diferencia.
unsigned long Trans2;
unsigned long Trans3;
unsigned long previousTrans1;
int vueltas = 0;   //variable que guarda el número de las vueltas
int volatile lectura;  //variable que recoge el valor del LDR
bool Flag = 0;  // Variable para evitar el rebote y que coja varios valores en un solo pase del vehículo

#include <Adafruit_NeoPixel.h>   //incluimos la librería Adafruit para la tira de leds 
#define PIN         14             // el pin conectado a la tira de leds 
#define NUMPIXELS    5         // Número de Leds conectados de la tira NeoPixels 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);  //instrucción que define los parámetros de la Tira de RGBs


//--------CODIGO HTML y JavaScript-------------
String webSite = "<!DOCTYPE html>"
                 "<html>"
                 "<head>"
                 "<meta charset='utf-8' />"
                 "<title>CronometroVelocistas</title>"
                 "<script type='text/javascript'>"  //el programa javascrip tiene que ir encerrado entre esto y script al final

                 "function loadDoc(){"   // crea la función cargar documento
                 "  var xhttp = new XMLHttpRequest();" //aquí crea la instancia xhttp que nos permite enviar y recibir información en formato XML
                 "  xhttp.onreadystatechange = function() {"
                 "    if (this.readyState == 4 && this.status == 200) {"
                 "      myFunction(this);"
                 "    }"
                 "  };"
                 "  xhttp.open('GET','xml',true);" //aquí definimos la petición  utilizando open
                 "  xhttp.send();" // y la enviamos
                 "  setTimeout('loadDoc()',500);"
                 "}"

                 "function myFunction(xml){"  // crea la función miFunción
                 //"  var i;"  //esto no hace nada
                 "  var xmlDoc1 = xml.responseXML;"  //En Response comprobaremos el contenidos
                 "  var dato1 ='';"
                 "  dato1 = xmlDoc1.getElementsByTagName('TEMPERATURA1')[0].childNodes[0].nodeValue;"  //Aquí no se puede poner xmlDoc1, etc
                 "  document.getElementById('temperatura1').innerHTML = dato1;"

                 //"  var i;"  // repito para el segundo dato
                 //"  var xmlDoc2 = xml.responseXML;"
                 "  var dato2 ='';"
                 "  dato2 = xmlDoc1.getElementsByTagName('TEMPERATURA2')[0].childNodes[0].nodeValue;"
                 "  document.getElementById('temperatura2').innerHTML = dato2;"

                 //"  var i;"  //repito para el tercer dato
                 //"  var xmlDoc3 = xml.responseXML;"
                 "  var dato3 ='';"
                 "  dato3 = xmlDoc1.getElementsByTagName('TEMPERATURA3')[0].childNodes[0].nodeValue;"
                 "  document.getElementById('temperatura3').innerHTML = dato3;"
                 "}"
                 "</script>"
                 "</head>"

                 // cierro cabecera // aquí empieza lo que voy a mostrar.

                 "<h2>TIEMPOS</h2>" // Titulo en la página
                 "<body onload='loadDoc()'>"
                 "<p><a>Primera Vuelta: </a>"  // Título delante del tiempo
                 "<a id='temperatura1'></a>"
                 "<a>ms</a></p>" //

                 "<body onload='loadDoc()'>"
                 "<a>Segunda Vuelta: </a>"  // Título delante del tiempo
                 "<a id='temperatura2'></a>"
                 "<a>ms</a></p>"  //

                 "<body onload='loadDoc()'>"
                 "<a>Tercera  Vuelta: </a>"  // Título delante del tiempo
                 "<a id='temperatura3'></a>"
                 "<a>ms</a></p>"

                 "</body>"
                 "</html>";


// FUNCIONES

void TiemposCrono()
{
  // Aquí calculo los tiempos
  // Serial.println(Trans1);  //y lo imprime, en milisegundos. en

  if (lectura >= 500)   //Cuando al sensor LDR le llega luz laser, pone Flag a cero.
  {
    Flag = 0;
  }
  if ((lectura <= 500) && ( vueltas == 0) && Flag == 0) // si alguien interrumpe el laser y el número de vueltas es 0, y Flag es 0
  {
    Flag = 1;  //pongo Flag a 1 para evitar que coja esta opción varias veces seguidas, sin dejar de interrumpir el laser
    Inicio = millis();  //  la variable inicio el tiempo de millis inicial
    delay(300); //  para evitar el rebote
    pixels.setPixelColor(0, pixels.Color(5, 0, 0)); // en la primera vuelta led primero Rojo
    pixels.show ();
    Serial.print("Primera Vuelta");  //Aparece la indicación de primera vuelta en la primera línea.
    vueltas = (vueltas + 1);  //Suma uno a las vueltas
    Serial.print("-- Tiempo --");     //se refleja la palabra tiempo para separar el título del dato siguiente de la primera línea
  }

  if ((lectura <= 500)  && (vueltas == 1) && Flag == 0)  // si el vehículo interrumpe el láser y ya se ha pasado una vez por delante y Flag es 0 estará dando la primera vuelta
  {
    Flag = 1;
    Tiempo1 = millis();  // adjudica a la variable Tiempo1 el valor de los milisegundos.
    delay(300); //  para evitar el rebote
    pixels.setPixelColor(1, pixels.Color(5, 0, 0));
    pixels.show ();
    Trans1 = Tiempo1 - Inicio;  // Calcula el tiempo de la primera vuelta.

    Serial.print(Trans1);  //y lo imprime, en milisegundos. en la primera fila.
    Serial.println(" ms");

    // HASTA AQUÍ LA PRIMERA VUELTA
    Serial.print("Segunda Vuelta");  // aparece en la segunda línea el texto de que está en la segunda vuelta.
    vueltas = (vueltas + 1);  // suma uno a las vueltas.
    Serial.print("-- Tiempo --");
  }

  if ((lectura <= 500)  && (vueltas == 2) && Flag == 0) //si el vehículo interrumpe el láser y ya se ha pasado dos veces por delante y Flag es 0 habrá dado una vuelta
  {
    Flag = 1;
    Tiempo2 = millis();
    delay(1000);
    pixels.setPixelColor(2, pixels.Color(5, 0, 0));
    pixels.show ();
    Trans2 = Tiempo2 - Tiempo1;
    Serial.print(Trans2);  //imprime los milisegudnos en la segunda línea
    Serial.println(" ms");

    // HASTA AQUÍ LA SEGUNDA VUELTA
    Serial.print("Tercera Vuelta");  // en la tercera fila
    vueltas = (vueltas + 1);
    Serial.print("-- Tiempo --");  // en la tercera fila
  }

  if ((lectura <= 500)  && (vueltas == 3) && Flag == 0)
  {
    Flag = 1;
    Tiempo3 = millis();
    delay(1000);
    vueltas = vueltas ++;
    Trans3 = Tiempo3 - Tiempo2;
    Serial.print(Trans3);
    Serial.print(" ms");
    Serial.println("   FINAL");

    vueltas = 0;  //pongo la variable vueltas a cero, para empezar de nuevo.
    Serial.println("Preparados, Listos, YA ");
    for (int k = 0; k < 3; k++) //Los leds por debajo e incluido  0ºC todos blancos
    {
      pixels.setPixelColor(k, pixels.Color(0, 5, 0));
      pixels.show ();
    }
  }
}


//--------Void Setup-------------

void setup()
{
  timer.attach(0.1, Salta);  // El temporizador timer va a la función Salta cada 100ms

  Serial.begin(115200);  // Monitor Serie
  Serial.println("");

  pixels.begin ();   // iniciamos la tira de leds
  pinMode(PIN, OUTPUT);  // el Pin lo definimos como salida
  pinMode(Laser, OUTPUT);  //definir pin salida Laser
  digitalWrite(Laser, HIGH);  //enciendo el laser para empezar. Si quiero empezar otra vez RESET

  for (int k = 0; k < 3; k++) //Al inicio todos los leds Verdes
  {
    pixels.setPixelColor(k, pixels.Color(0, 5, 0));
    pixels.show ();
  }

  // Conexión WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED and contconexion < 50) { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(500);
    Serial.print(".");  // mientras se conecta van apareciendo puntos en Monitor Serie cada 0,5 segundos
  }
  if (contconexion < 50) {

    //IP para usar 
    IPAddress ip(192, 168, 1, 156); //192.168.1.100
    IPAddress gateway(192, 168, 1, 1);

    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);

    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Error de conexion");
  }

  server.on("/", handleWebsite);
  server.on("/xml", handleXML); // aquí no se puede handleXML1 etc
  server.begin();
}

//--------Void Loop-------------

void loop()
{

  // CADA SEGUNDO ENVÍA DATOS A LA WEB
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) { //envia la temperatura cada 1 segundos
    previousMillis = currentMillis;
    float temp1 = Trans1;   // Se supone que lanzo estos tres datos a la Web.
    float temp2 = Trans2;
    float temp3 = Trans3;
    xmlTemperatura1 = String(temp1, 0); //1 decimal
    xmlTemperatura2 = String(temp2, 0); //1 decimal
    xmlTemperatura3 = String(temp3, 0); //1 decimal
  }

  server.handleClient(); // tiene que estar porque si no no conceta

  // AQUÍ CALCULA LOS TIEMPOS DEL CRONÓMETRO Y LOS IMPRIME en el monitor serie

  TiemposCrono();  // Aquí llamo a la función TiemposCrono para que capture los valores de las variables.
}


//--------OTRAS FUNCIONES-------------

void Salta()  // FUNCIÓN CON LA QUE EL TIMER CAPTURA EL VALOR DEL SENSOR LDR CADA 100ms
{
  lectura = analogRead(A0);
}

void handleWebsite() {
  server.send(200, "text/html", webSite);
}

void handleXML() {
  construirXML();
  server.send(200, "text/xml", XML1);
  //server.send(200,"text/xml",XML2);
  //server.send(200,"text/xml",XML3);
}
void construirXML() {
  XML1 = "";
  XML1 += "<TEMPERATURA1>";
  XML1 += xmlTemperatura1; //aquí añado otros dos xmlTemperatura
  XML1 += "</TEMPERATURA1>";

  //XML1="";
  XML1 += "<TEMPERATURA2>";
  XML1 += xmlTemperatura2;
  XML1 += "<TEMPERATURA2>";

  //XML1="";
  XML1 += "<TEMPERATURA3>";
  XML1 += xmlTemperatura3;
  XML1 += "<TEMPERATURA1>";
}
