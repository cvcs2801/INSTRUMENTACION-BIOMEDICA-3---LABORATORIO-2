/*
 * Instrumentación Biomédica III - Guía de Laboratorio N.º 1
 * Escuela Profesional de Ingeniería Biomédica - UNMSM
 *
 * Simulador de electrodo de pH con el ESP32 (Sección 5 de la guía)
 * Versión para Arduino IDE.
 *
 * Requisitos que cumple este programa:
 *  - Lee desde el Monitor Serial un valor de pH (0 a 14).
 *  - Calcula el voltaje según la ecuación de Nernst (Sección 2.2),
 *    con un factor de escala para que la variación sea visible en
 *    el osciloscopio.
 *  - Limita el voltaje al rango válido del DAC del ESP32 (0 a 3.3 V).
 *  - Entrega ese voltaje en GPIO25 mediante el DAC interno (dacWrite).
 *  - Muestra en el Monitor Serial el pH ingresado y el voltaje entregado.
 *
 * Placa: "ESP32 Dev Module" (u otra ESP32 clásica). dacWrite() solo
 * funciona en GPIO25 y GPIO26 del ESP32 original (no en S2/S3/C3).
 */

#define PIN_DAC 25   // DAC1 = GPIO25

// Ecuación de Nernst (Sección 2.2): E = E0 - PENDIENTE * (pH - 7)
const float PENDIENTE_NERNST = 0.05916f;   // V por unidad de pH

// Offset y factor de escala para centrar la señal y hacerla visible
// en el osciloscopio (la variación "cruda" de Nernst es de ~60 mV/pH).
const float E0_OFFSET_V   = 1.65f;   // voltaje a pH = 7 (mitad del DAC)
const float FACTOR_ESCALA = 4.0f;    // amplifica la variación de Nernst

// Límites físicos del DAC del ESP32
const float VDAC_MIN_V = 0.0f;
const float VDAC_MAX_V = 3.3f;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== Simulador de electrodo de pH - ESP32 (GPIO25 / DAC) ===");
  Serial.println("Guia de Laboratorio N.1 - Instrumentacion Biomedica III - UNMSM");
  Serial.println("Ingrese un valor de pH entre 0 y 14 y presione Enter.");
  Serial.println();
}

void loop() {
  Serial.print("pH > ");

  // Esperar a que llegue una línea completa por el Monitor Serial
  while (!Serial.available()) {
    delay(10);
  }
  String linea = Serial.readStringUntil('\n');
  linea.trim();

  if (linea.length() == 0) {
    return;   // Enter vacío: vuelve a pedir
  }

  // Validar que el texto realmente empiece como un número
  char primero = linea.charAt(0);
  bool parece_numero = isDigit(primero) || primero == '-' || primero == '+' || primero == '.';
  if (!parece_numero) {
    Serial.println("Entrada no valida. Escriba un numero entre 0 y 14.");
    Serial.println();
    return;
  }

  float ph = linea.toFloat();

  if (ph < 0.0f || ph > 14.0f) {
    Serial.println("Fuera de rango. El pH debe estar entre 0 y 14.");
    Serial.println();
    return;
  }

  // Ecuación de Nernst con offset y factor de escala
  float voltaje = E0_OFFSET_V - FACTOR_ESCALA * PENDIENTE_NERNST * (ph - 7.0f);

  // Limitar al rango válido del DAC del ESP32 (0 a 3.3 V)
  bool saturado = false;
  if (voltaje > VDAC_MAX_V) {
    voltaje = VDAC_MAX_V;
    saturado = true;
  } else if (voltaje < VDAC_MIN_V) {
    voltaje = VDAC_MIN_V;
    saturado = true;
  }

  // Convertir el voltaje a un valor digital de 8 bits (0-255)
  uint8_t valor_dac = (uint8_t)((voltaje / VDAC_MAX_V) * 255.0f + 0.5f);
  dacWrite(PIN_DAC, valor_dac);

  Serial.print("pH ingresado: ");
  Serial.print(ph, 2);
  Serial.print("  ->  Voltaje entregado en GPIO25: ");
  Serial.print(voltaje, 4);
  Serial.print(" V (valor DAC = ");
  Serial.print(valor_dac);
  Serial.print(")");
  if (saturado) {
    Serial.print("  [LIMITADO AL RANGO DEL DAC]");
  }
  Serial.println();
  Serial.println();
}
