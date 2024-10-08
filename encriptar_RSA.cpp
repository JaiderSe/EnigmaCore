#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

void encryptAESKeyAndIV(const std::string& public_key_path, const unsigned char* aes_key, const unsigned char* iv, std::ofstream& outputFile) {
    // Abre el archivo de la llave pública
    FILE* pubKeyFile = fopen(public_key_path.c_str(), "rb");
    if (!pubKeyFile) {
        std::cerr << "[ERROR] No se pudo abrir el archivo de la llave publica: " << public_key_path << std::endl;
        return;
    }

    EVP_PKEY* evp_pkey = PEM_read_PUBKEY(pubKeyFile, nullptr, nullptr, nullptr);
    fclose(pubKeyFile);

    if (!evp_pkey) {
        std::cerr << "❌ [ERROR] No se pudo leer la llave publica RSA." << std::endl;
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
void decryptAESKeyAndIV(const std::string& private_key_path, const unsigned char* encrypted_aes_key, const unsigned char* encrypted_iv, unsigned char* aes_key, unsigned char* iv) {
    // Abre el archivo de la llave privada
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

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, aes_key, &outlen, encrypted_aes_key, EVP_PKEY_size(evp_pkey)) <= 0) {
        std::cerr << "❌ [ERROR] Error desencriptando la clave AES." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    if (EVP_PKEY_decrypt(ctx, iv, &outlen, encrypted_iv, EVP_PKEY_size(evp_pkey)) <= 0) {
        std::cerr << "❌ [ERROR] Error desencriptando el IV." << std::endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(evp_pkey);
        return;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(evp_pkey);
}

int main() {
    std::vector<unsigned char> key(32);
    std::vector<unsigned char> iv(16);
    
    RAND_bytes(key.data(), key.size());
    RAND_bytes(iv.data(), iv.size());
    
    std::ofstream encrypted_file("data/KEYS/aencrypted_data.bin", std::ios::binary);
    if (!encrypted_file) {
        std::cerr << "❌ [ERROR] No se pudo abrir el archivo para escribir." << std::endl;
        return 1;
    }

    encryptAESKeyAndIV("data/KEYS/public_key.pem", key.data(), iv.data(), encrypted_file);
    encrypted_file.close();

    return 0;
}
