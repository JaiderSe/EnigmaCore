# Usa una imagen base con soporte para C++
FROM gcc:latest

# Instala OpenSSL y sus dependencias
RUN apt-get update && apt-get install -y \
    libssl-dev \
    build-essential

# Copia el código fuente al contenedor
WORKDIR /app
COPY src/main.cpp /app

# Compila el código
RUN g++ -o app main.cpp -lssl -lcrypto

# Define el punto de entrada para el contenedor
ENTRYPOINT ["/app/app"]
