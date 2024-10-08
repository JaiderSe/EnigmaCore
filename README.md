## 🚀 Codefest-AD-Astra 2024

## 📜 Descripción

Codefest-AD-Astra 2024 es una competencia de desarrollo de software organizada por la Fuerza Aérea Colombiana. Este
proyecto tiene como objetivo desarrollar una aplicación en C++ para abordar un desafío específico en el ámbito de la
tecnología y la innovación. La aplicación está diseñada para realizar la encriptación y desencriptación de archivos
utilizando el algoritmo AES-256 en modo CTR, garantizando así la seguridad y confidencialidad de los datos.

El proyecto hace uso de Docker para la contenerización, lo que permite asegurar un entorno de desarrollo homogéneo y
simplifica la implementación en producción. Además, se emplea OpenSSL para implementar funciones de criptografía
avanzadas, como el cifrado y descifrado de datos, asegurando que la aplicación sea robusta y confiable.

Nota: La aplicación puede manejar cualquier tipo de dato, no está limitada a archivos específicos como imágenes o
textos.

## 🛠️ Tecnologías

- **C++**: Lenguaje de programación principal utilizado para desarrollar la lógica de la aplicación.
- **Docker**: Herramienta para crear contenedores que aseguran que la aplicación funcione de manera consistente en
  diferentes entornos.
- **Docker Compose**: Utilizado para definir y ejecutar aplicaciones Docker de múltiples contenedores, facilitando la
  gestión de servicios relacionados.
- **OpenSSL**: Biblioteca utilizada para implementar funciones de criptografía y seguridad en la aplicación.

## 📁 Estructura del Proyecto

 ```text
   /Codefest-AD-Astra-2024
├── app.cpp
├── Documento.pdf
├── Dockerfile
├── docker-compose.yml
├── README.md
├── video
│   └── video.mp4
└── data
    └── 5.NEF
└── main.cpp
   ```

**🚀 Instalación:**

**Clonar el repositorio:**

   ```bash
   git clone https://github.com/JaiderSe/Codefest-AD-Astra-Final-1.git
   ```

Redirigir al directorio del proyecto:

   ```bash
   cd codefest-ad-astra-2024
   ```

## Compilar el Código

Compila el código fuente utilizando el compilador C++ y las bibliotecas de OpenSSL. Asegúrate de que el archivo main.cpp
esté en el directorio actual:

   ```bash
   g++ -o app src/main.cpp -lssl -lcrypto
   ```

## 🖥️ Ejecutar la Aplicación

Ejecuta la aplicación compilada. Asegúrate de que el archivo 5.NEF esté presente en el directorio data:

  ```bash
    ./app <operation> <input_path> <output_path>
  ```

o

 ```bash
    app <operation> <input_path> <output_path>
  ```

Ejemplo de uso para encriptar:

 ```bash
./app encrypt data/5.NEF data/encrypt/image_encrypted.bin
 ```

o

 ```bash
app encrypt data/5.NEF data/encrypt/image_encrypted.bin
 ```

Ejemplo de uso para desencriptar:

  ```bash
  ./app decrypt data/encrypt/image_encrypted.bin data/decrypt/image_decrypted.NEF
  ```

o

  ```bash
  app decrypt data/encrypt/image_encrypted.bin data/decrypt/image_decrypted.NEF
  ```

## 🐳 Construir y Ejecutar la Imagen de Docker

   ```bash
   docker-compose build
   ```

**Ejecutar la Aplicación**

  ```bash
   docker-compose up
   ```

**🐧 En Linux**

Si estás en Linux, puedes usar el mismo comando. Sin embargo, si tienes problemas con permisos, puedes necesitar
ejecutar el comando con sudo:

  ```bash
   sudo docker-compose up
   ```

**⛔ Detener la Aplicación**

Para detener la aplicación que se está ejecutando con Docker Compose, puedes presionar Ctrl + C en la terminal. Si
deseas eliminar los contenedores y redes creados por Docker Compose, puedes usar:

  ```bash
   docker-compose down
   ```

# Instalación de MSYS2 y OpenSSL en Windows

## 1. Descargar e Instalar MSYS2

1. **Descargar el Instalador:**

- Visita la página de [descarga de MSYS2](https://www.msys2.org/) y descarga el instalador adecuado para tu sistema (por
  ejemplo, `msys2-x86_64-<version>.exe`).

2. **Ejecutar el Instalador:**

- Ejecuta el archivo descargado y sigue las instrucciones del asistente de instalación.
- Elige la ubicación de instalación (por defecto suele ser `C:\msys64`).

3. **Actualizar el Sistema:**

- Abre el terminal de MSYS2 (puedes encontrarlo en el menú de inicio como "MSYS2 MSYS").
- Ejecuta los siguientes comandos para actualizar el sistema y los paquetes:
  ```bash
  pacman -Syu
  ```

- Si el sistema solicita reiniciar el terminal, hazlo y luego ejecuta:
  ```bash
  pacman -Syu
  ```

## 2. Instalar el Compilador y Herramientas Básicas

1. **Instalar el Compilador GCC y Herramientas Básicas:**

- En el terminal de MSYS2, ejecuta:
  ```bash
  pacman -S base-devel mingw-w64-x86_64-toolchain
  ```

## 3. Instalar OpenSSL

1. **Instalar OpenSSL:**

- Ejecuta el siguiente comando en el terminal de MSYS2:
  ```bash
  pacman -S mingw-w64-x86_64-openssl
  ```

## 4. Configurar el Entorno

1. **Agregar Directorios al PATH:**

- Agrega las rutas de OpenSSL a las variables de entorno. Puedes hacerlo desde el terminal de MSYS2 ejecutando:
  ```bash
  export PATH=$PATH:/c/msys64/mingw64/bin
  ```

2. **Verificar la Instalación:**

- Verifica que OpenSSL esté instalado correctamente ejecutando:
  ```bash
  openssl version
  ```
- Deberías ver la versión de OpenSSL que se ha instalado.

## Archivos de prueba

Puede crear archivos de prueba con el siguiente comando

  ```bash
  fsutil file createnew mifichero.txt 1073741824
  ```

https://www.mundodeportivo.com/urbantecno/windows/crear-ficheros-pruebas

## 👥 Participantes

DELGADO DIAZ DIEGO ARLEY <dadelgadod@udistrital.edu.co>

VILLALBA BELTRAN CAREN DANIELA <cdvillalbab@udistrital.edu.co>

JAIDER SEBASTIAN MORENO QUINTERO <jsmorenoq@udistrital.edu.co>

NICOLAS ENRIQUE RUIZ VEGA <neruizv@udistrital.edu.co>

