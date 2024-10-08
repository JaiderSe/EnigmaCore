#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <openssl/rsa.h>
#include <openssl/pem.h>


// Función para manejar errores de OpenSSL
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

// Función para cifrar y descifrar datos usando AES-CTR
// Esta función toma como entrada los datos que se desean cifrar/descifrar, una clave (key),
// un vector de inicialización (iv), y genera la salida correspondiente en la variable 'output'.
// El parámetro 'encrypt' define si la operación es de cifrado (true) o descifrado (false).
void aesCrypt(const unsigned char *input, int input_len, unsigned char *key, unsigned char *iv, unsigned char *output,
              bool encrypt) {
    // Crear y inicializar el contexto de cifrado/descifrado
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors(); // Manejar errores si la creación del contexto falla

    // Seleccionar el cifrado AES en modo CTR
    const EVP_CIPHER *cipher = EVP_aes_256_ctr();

    // Inicializar el contexto dependiendo de si se va a cifrar o descifrar
    if (encrypt) {
        // Inicialización para cifrado
        if (1 != EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv)) handleErrors();
    } else {
        // Inicialización para descifrado
        if (1 != EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv)) handleErrors();
    }

    int len; // Variable para almacenar el tamaño de los datos procesados

    // Actualizar el contexto con los datos de entrada y obtener la salida cifrada/descifrada
    if (1 != (encrypt
                  ? EVP_EncryptUpdate(ctx, output, &len, input, input_len) // Cifrar datos
                  : EVP_DecryptUpdate(ctx, output, &len, input, input_len))) // Descifrar datos
        handleErrors(); // Manejar errores si la operación falla

    // Liberar el contexto de cifrado/descifrado
    EVP_CIPHER_CTX_free(ctx);
}

// Función para formatear el tamaño en bytes a una representación legible
std::string formatBytes(size_t bytes) {
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;

    std::ostringstream oss;
    if (bytes < KB) {
        oss << std::fixed << std::setprecision(2) << bytes << " bytes";
    } else if (bytes < MB) {
        oss << std::fixed << std::setprecision(2) << bytes / KB << " KB";
    } else if (bytes < GB) {
        oss << std::fixed << std::setprecision(2) << bytes / MB << " MB";
    } else {
        oss << std::fixed << std::setprecision(2) << bytes / GB << " GB";
    }
    return oss.str();
}

// Función para mostrar la barra de progreso
void showProgress(size_t current, size_t total, size_t updateInterval = 1024) {
    static size_t lastUpdate = 0; // Última actualización
    static auto lastUpdateTime = std::chrono::steady_clock::now(); // Hora de la última actualización

    // Calcula el progreso
    float progress = (float) current / total;
    int barWidth = 50;
    int pos = barWidth * progress;

    // Solo actualiza si se han procesado suficientes bytes o ha pasado suficiente tiempo
    if (current - lastUpdate >= updateInterval || std::chrono::steady_clock::now() - lastUpdateTime >
        std::chrono::milliseconds(500)) {
        std::cout << "\r[" << std::string(pos, '#') << std::string(barWidth - pos, ' ') << "] "
                << int(progress * 100.0) << "% "
                << std::flush;

        lastUpdate = current;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
}

std::string formatDuration(double seconds) {
    std::ostringstream oss;

    if (seconds < 1e-6) {
        // Menos de 1 microsegundo
        oss << std::fixed << std::setprecision(2) << seconds * 1e9 << " ns"; // Nanosegundos
    } else if (seconds < 1e-3) {
        // Menos de 1 milisegundo
        oss << std::fixed << std::setprecision(2) << seconds * 1e6 << " µs"; // Microsegundos
    } else if (seconds < 1.0) {
        // Menos de 1 segundo
        oss << std::fixed << std::setprecision(2) << seconds * 1e3 << " ms"; // Milisegundos
    } else if (seconds < 60.0) {
        // Menos de 1 minuto
        oss << std::fixed << std::setprecision(2) << seconds << " s"; // Segundos
    } else if (seconds < 3600.0) {
        // Menos de 1 hora
        int minutes = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        oss << minutes << " min " << secs << " s"; // Minutos y segundos
    } else {
        // 1 hora o más
        int hours = static_cast<int>(seconds) / 3600;
        int minutes = (static_cast<int>(seconds) % 3600) / 60;
        int secs = static_cast<int>(seconds) % 60;
        oss << hours << " h " << minutes << " min " << secs << " s"; // Horas, minutos y segundos
    }

    return oss.str();
}

// Función para imprimir los resultados del proceso
void printFormattedResults(size_t originalSize, size_t processedSize, double duration) {
    std::cout << "\n--- Resumen del Proceso ---" << std::endl;
    std::cout << "Tamaño del archivo original: " << formatBytes(originalSize) << std::endl; // Tamaño original
    std::cout << "Tamaño del archivo procesado: " << formatBytes(processedSize) << std::endl; // Tamaño procesado
    std::cout << "Diferencia de tamaño: " << formatBytes(
        std::abs(static_cast<long long>(processedSize) - static_cast<long long>(originalSize))) << std::endl;
    // Diferencia de tamaño
    std::cout << "Tiempo total: " << formatDuration(duration) << std::endl; // Tiempo total
    std::cout << "----------------------------\n" << std::endl;
}

// Función para encriptar la llave AES y el IV
void encryptAESKeyAndIV(const std::string& public_key_path, unsigned char* aes_key, unsigned char* iv, std::ofstream& outputFile) {
    FILE* pubKeyFile = fopen(public_key_path.c_str(), "rb");
    if (!pubKeyFile) {
        std::cerr << "❌ [ERROR] No se pudo abrir el archivo de la llave pública: " << public_key_path << std::endl;
        return;
    }

    EVP_PKEY* evp_pkey = PEM_read_PUBKEY(pubKeyFile, nullptr, nullptr, nullptr);
    fclose(pubKeyFile);

    if (!evp_pkey) {
        std::cerr << "❌ [ERROR] No se pudo leer la llave pública RSA." << std::endl;
        return;
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_pkey, nullptr);
    if (!ctx) {
        std::cerr << "❌ [ERROR] Error creando el contexto de la llave pública." << std::endl;
        EVP_PKEY_free(evp_pkey);
        return;
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        std::cerr << "❌ [ERROR] Error inicializando la encriptación." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    // Establecer el esquema de padding
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        std::cerr << "❌ [ERROR] Error estableciendo el padding." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    size_t outlen;
    std::vector<unsigned char> encrypted_aes_key(EVP_PKEY_size(evp_pkey));
    if (EVP_PKEY_encrypt(ctx, encrypted_aes_key.data(), &outlen, aes_key, 32) <= 0) {
        std::cerr << "❌ [ERROR] Error encriptando la clave AES." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    encrypted_aes_key.resize(outlen);
    outputFile.write(reinterpret_cast<char*>(encrypted_aes_key.data()), outlen);

    std::vector<unsigned char> encrypted_iv(EVP_PKEY_size(evp_pkey));
    if (EVP_PKEY_encrypt(ctx, encrypted_iv.data(), &outlen, iv, 16) <= 0) {
        std::cerr << "❌ [ERROR] Error encriptando el IV." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    encrypted_iv.resize(outlen);
    outputFile.write(reinterpret_cast<char*>(encrypted_iv.data()), outlen);

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(evp_pkey);
}

// Función para desencriptar la llave AES y el IV
void decryptAESKeyAndIV(const std::string& private_key_path, unsigned char* aes_key, unsigned char* iv, std::ifstream& inputFile) {
    FILE* privKeyFile = fopen(private_key_path.c_str(), "rb");
    if (!privKeyFile) {
        std::cerr << "❌ [ERROR] No se pudo abrir el archivo de la llave privada: " << private_key_path << std::endl;
        return;
    }

    EVP_PKEY* evp_pkey = PEM_read_PrivateKey(privKeyFile, nullptr, nullptr, nullptr);
    fclose(privKeyFile);

    if (!evp_pkey) {
        std::cerr << "❌ [ERROR] No se pudo leer la llave privada RSA." << std::endl;
        return;
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_pkey, nullptr);
    if (!ctx) {
        std::cerr << "❌ [ERROR] Error creando el contexto de la llave privada." << std::endl;
        EVP_PKEY_free(evp_pkey);
        return;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        std::cerr << "❌ [ERROR] Error inicializando la desencriptación." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    // Establecer el esquema de padding
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        std::cerr << "❌ [ERROR] Error estableciendo el padding." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    std::vector<unsigned char> encrypted_aes_key(EVP_PKEY_size(evp_pkey));
    inputFile.read(reinterpret_cast<char*>(encrypted_aes_key.data()), encrypted_aes_key.size());

    size_t outlen = 32;  // Tamaño de la clave AES
    if (EVP_PKEY_decrypt(ctx, aes_key, &outlen, encrypted_aes_key.data(), encrypted_aes_key.size()) <= 0) {
        std::cerr << "❌ [ERROR] Error desencriptando la clave AES." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    std::vector<unsigned char> encrypted_iv(EVP_PKEY_size(evp_pkey));
    inputFile.read(reinterpret_cast<char*>(encrypted_iv.data()), encrypted_iv.size());

    outlen = 16;  // Tamaño del IV
    if (EVP_PKEY_decrypt(ctx, iv, &outlen, encrypted_iv.data(), encrypted_iv.size()) <= 0) {
        std::cerr << "❌ [ERROR] Error desencriptando el IV." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(evp_pkey);
}

// Función para cifrar un archivo
void encrypt(const std::string &input_path, const std::string &output_path) {
    // Mostrar las rutas de los archivos de entrada y salida
    std::cout << "input_path=" << input_path << std::endl;
    std::cout << "output_path=" << output_path << std::endl;

    // Crear el directorio de salida si no existe
    std::filesystem::path outputFilePath(output_path);
    std::filesystem::path outputDir = outputFilePath.parent_path();
    if (!std::filesystem::exists(outputDir)) {
        // Si el directorio de salida no existe, intentar crearlo
        if (!std::filesystem::create_directories(outputDir)) {
            std::cerr << "❌ [ERROR] No se pudo crear el directorio: " << outputDir << std::endl;
            return; // Salir si la creación del directorio falla
        }
    }

    // Abrir el archivo de entrada en modo binario
    std::ifstream inputFile(input_path, std::ios::binary);
    if (!inputFile) {
        std::cerr << "❌ [ERROR] No se pudo abrir el archivo: " << input_path << std::endl;
        return; // Salir si no se puede abrir el archivo de entrada
    }

    // Crear el archivo de salida en modo binario
    std::ofstream outputFile(output_path, std::ios::binary);
    if (!outputFile) {
        std::cerr << "❌ [ERROR] No se pudo crear el archivo de salida: " << output_path << std::endl;
        return; // Salir si no se puede crear el archivo de salida
    }

    // Preparar un buffer para leer el archivo en bloques
    std::vector<unsigned char> buffer(4096);
    unsigned char key[32], iv[16];
    // Generar la clave y el vector de inicialización (IV) aleatorios
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));

    encryptAESKeyAndIV("C:\\Users\\User\\Documents\\GitHub\\Codefest-AD-Astra-Final-1\\data\\KEYS\\public_key.bin", key, iv, outputFile);

    // Guardar la clave y el IV en el archivo de salida (por ejemplo, al principio del archivo)
    // outputFile.write(reinterpret_cast<char*>(key), sizeof(key));
    // outputFile.write(reinterpret_cast<char*>(iv), sizeof(iv));

    size_t totalBytesRead = 0; // Contador de bytes leídos
    size_t fileSize = std::filesystem::file_size(input_path); // Obtener el tamaño total del archivo

    // Leer el archivo en bloques y cifrar cada bloque
    while (inputFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size()) || inputFile.gcount() > 0) {
        std::vector<unsigned char> outputBuffer(inputFile.gcount());
        // Cifrar los datos leídos en el buffer
        aesCrypt(buffer.data(), inputFile.gcount(), key, iv, outputBuffer.data(), true);
        // Escribir los datos cifrados en el archivo de salida
        outputFile.write(reinterpret_cast<char *>(outputBuffer.data()), inputFile.gcount());
        totalBytesRead += inputFile.gcount(); // Actualizar el contador de bytes leídos

        // Comentar la siguiente línea para habilitar la barra de progreso opcional
        // showProgress(totalBytesRead, fileSize);
    }

    // Imprimir un mensaje indicando que el proceso de cifrado ha finalizado
    std::cout << std::endl;
    std::cout << "Encrypted image" << std::endl;
}

// Función para descifrar un archivo
void decrypt(const std::string &input_path, const std::string &output_path) {
    // Mostrar las rutas de los archivos de entrada y salida
    std::cout << "input_path=" << input_path << std::endl;
    std::cout << "output_path=" << output_path << std::endl;

    // Crear el directorio de salida si no existe
    std::filesystem::path outputFilePath(output_path);
    std::filesystem::path outputDir = outputFilePath.parent_path();
    if (!std::filesystem::exists(outputDir)) {
        // Si el directorio de salida no existe, intentar crearlo
        if (!std::filesystem::create_directories(outputDir)) {
            std::cerr << "❌ [ERROR] No se pudo crear el directorio: " << outputDir << std::endl;
            return; // Salir si la creación del directorio falla
        }
    }

    // Abrir el archivo de entrada en modo binario
    std::ifstream inputFile(input_path, std::ios::binary);
    if (!inputFile) {
        std::cerr << "❌ [ERROR] No se pudo abrir el archivo: " << input_path << std::endl;
        return; // Salir si no se puede abrir el archivo de entrada
    }

    // Crear el archivo de salida en modo binario
    std::ofstream outputFile(output_path, std::ios::binary);
    if (!outputFile) {
        std::cerr << "❌ [ERROR] No se pudo crear el archivo de salida: " << output_path << std::endl;
        return; // Salir si no se puede crear el archivo de salida
    }

    // Preparar un buffer para leer el archivo en bloques
    std::vector<unsigned char> buffer(4096);
    unsigned char key[32], iv[16];
    // leer la clave AES y el iv
    decryptAESKeyAndIV("C:\\Users\\User\\Documents\\GitHub\\Codefest-AD-Astra-Final-1\\data\\KEYS\\private_key.bin", key, iv, inputFile);

    size_t totalBytesRead = 0; // Contador de bytes leídos
    size_t fileSize = std::filesystem::file_size(input_path); // Obtener el tamaño total del archivo

    // Leer el archivo en bloques y descifrar cada bloque
    while (inputFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size()) || inputFile.gcount() > 0) {
        std::vector<unsigned char> outputBuffer(inputFile.gcount());
        // Descifrar los datos leídos en el buffer
        aesCrypt(buffer.data(), inputFile.gcount(), key, iv, outputBuffer.data(), false);
        // Escribir los datos descifrados en el archivo de salida
        outputFile.write(reinterpret_cast<char *>(outputBuffer.data()), inputFile.gcount());
        totalBytesRead += inputFile.gcount(); // Actualizar el contador de bytes leídos

        // Comentar la siguiente línea para habilitar la barra de progreso opcional
        // showProgress(totalBytesRead, fileSize);
    }

    std::cout << std::endl;
    std::cout << "Decrypted image" << std::endl;
}

int main(int argc, char *argv[]) {
    // Verifica que el número de argumentos sea exactamente 4 (nombre del programa + 3 argumentos)
    if (argc != 4) {
        // Muestra el uso correcto del programa si los argumentos son incorrectos
        std::cerr << "Uso: " << argv[0] << " <operation> <input_path> <output_path>" << std::endl;
        return 1;
    }

    // Asigna los argumentos de la línea de comandos a variables de string para facilidad de uso
    std::string operation = argv[1];
    std::string input_path = argv[2];
    std::string output_path = argv[3];

    // Inicia un temporizador para medir la duración de la operación
    auto start = std::chrono::high_resolution_clock::now();

    // Verifica qué operación debe realizarse: cifrar o descifrar
    if (operation == "encrypt") {
        // Si la operación es "encrypt", llama a la función de cifrado
        encrypt(input_path, output_path);
    } else if (operation == "decrypt") {
        // Si la operación es "decrypt", llama a la función de descifrado
        decrypt(input_path, output_path);
    } else {
        // Si la operación no es válida, muestra un mensaje de error y termina el programa
        std::cerr << "Operación no válida: " << operation << std::endl;
        return 1; // Devuelve 1 para indicar un error en la ejecución
    }

    // Detiene el temporizador y calcula la duración de la operación
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Obtiene el tamaño de los archivos de entrada y salida
    size_t processedFileSize = std::filesystem::file_size(output_path);
    size_t originalFileSize = std::filesystem::file_size(input_path);

    // Imprime información detallada del proceso (descomentando la siguiente línea)
    // printFormattedResults(originalFileSize, processedFileSize, duration.count());

    return 0;
}
