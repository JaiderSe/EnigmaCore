#include <iostream>
#include <fstream>

int main() {
    // Intenta abrir el archivo en modo binario
    std::ifstream file("C:\\Users\\User\\Documents\\GitHub\\Codefest-AD-Astra-Final-1\\data\\KEYS\\private_key.bin", std::ios::binary);

    // Verifica si el archivo se abrió correctamente
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo" << std::endl;
        return 1;
    }

    // Lee el contenido del archivo
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Muestra el tamaño del archivo (en bytes)
    std::cout << "Tamaño del archivo: " << content.size() << " bytes" << std::endl;

    // Cierra el archivo
    file.close();

    return 0;
}
