#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <direct.h>
#include <sys/stat.h>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Nodo {
    string palabra_es;
    map<string, string> traducciones;
    Nodo* izquierdo;
    Nodo* derecho;
    int altura;

    Nodo(string es, string it, string fr, string de, string en) {
        palabra_es = es;
        traducciones["italiano"] = it;
        traducciones["frances"] = fr;
        traducciones["aleman"] = de;
        traducciones["ingles"] = en;
        izquierdo = nullptr;
        derecho = nullptr;
        altura = 1;
    }

    ~Nodo() {
        delete izquierdo;
        delete derecho;
    }
};

vector<string> historial;
vector<string> historial_encriptado;
map<string, int> contadorPalabras;
string usuarioActual;

int altura(Nodo* nodo) {
    if (nodo == nullptr)
        return 0;
    return nodo->altura;
}

int maximo(int a, int b) {
    return (a > b) ? a : b;
}

int obtenerBalance(Nodo* nodo) {
    if (nodo == nullptr)
        return 0;
    return altura(nodo->izquierdo) - altura(nodo->derecho);
}

Nodo* rotacionDerecha(Nodo* y) {
    Nodo* x = y->izquierdo;
    Nodo* T2 = x->derecho;

    x->derecho = y;
    y->izquierdo = T2;

    y->altura = maximo(altura(y->izquierdo), altura(y->derecho)) + 1;
    x->altura = maximo(altura(x->izquierdo), altura(x->derecho)) + 1;

    return x;
}

Nodo* rotacionIzquierda(Nodo* x) {
    Nodo* y = x->derecho;
    Nodo* T2 = y->izquierdo;

    y->izquierdo = x;
    x->derecho = T2;

    x->altura = maximo(altura(x->izquierdo), altura(x->derecho)) + 1;
    y->altura = maximo(altura(y->izquierdo), altura(y->derecho)) + 1;

    return y;
}

Nodo* insertarNodo(Nodo* nodo, string es, string it, string fr, string de, string en) {
    if (nodo == nullptr) {
        nodo = new Nodo(es, it, fr, de, en);
        return nodo;
    }

    if (es < nodo->palabra_es)
        nodo->izquierdo = insertarNodo(nodo->izquierdo, es, it, fr, de, en);
    else if (es > nodo->palabra_es)
        nodo->derecho = insertarNodo(nodo->derecho, es, it, fr, de, en);
    else
        return nodo;

    nodo->altura = 1 + maximo(altura(nodo->izquierdo), altura(nodo->derecho));

    int balance = obtenerBalance(nodo);

    if (balance > 1 && es < nodo->izquierdo->palabra_es)
        return rotacionDerecha(nodo);

    if (balance < -1 && es > nodo->derecho->palabra_es)
        return rotacionIzquierda(nodo);

    if (balance > 1 && es > nodo->izquierdo->palabra_es) {
        nodo->izquierdo = rotacionIzquierda(nodo->izquierdo);
        return rotacionDerecha(nodo);
    }

    if (balance < -1 && es < nodo->derecho->palabra_es) {
        nodo->derecho = rotacionDerecha(nodo->derecho);
        return rotacionIzquierda(nodo);
    }

    return nodo;
}

Nodo* encontrarNodoMinimo(Nodo* nodo) {
    Nodo* actual = nodo;
    while (actual->izquierdo != nullptr)
        actual = actual->izquierdo;
    return actual;
}

Nodo* eliminarNodo(Nodo* raiz, string palabra_es) {
    if (raiz == nullptr)
        return raiz;

    if (palabra_es < raiz->palabra_es)
        raiz->izquierdo = eliminarNodo(raiz->izquierdo, palabra_es);
    else if (palabra_es > raiz->palabra_es)
        raiz->derecho = eliminarNodo(raiz->derecho, palabra_es);
    else {
        if (raiz->izquierdo == nullptr || raiz->derecho == nullptr) {
            Nodo* temp = raiz->izquierdo ? raiz->izquierdo : raiz->derecho;

            if (temp == nullptr) {
                temp = raiz;
                raiz = nullptr;
            } else
                *raiz = *temp;
            delete temp;
        } else {
            Nodo* temp = encontrarNodoMinimo(raiz->derecho);
            raiz->palabra_es = temp->palabra_es;
            raiz->traducciones = temp->traducciones;
            raiz->derecho = eliminarNodo(raiz->derecho, temp->palabra_es);
        }
    }

    if (raiz == nullptr)
        return raiz;

    raiz->altura = 1 + maximo(altura(raiz->izquierdo), altura(raiz->derecho));

    int balance = obtenerBalance(raiz);

    if (balance > 1 && obtenerBalance(raiz->izquierdo) >= 0)
        return rotacionDerecha(raiz);

    if (balance > 1 && obtenerBalance(raiz->izquierdo) < 0) {
        raiz->izquierdo = rotacionIzquierda(raiz->izquierdo);
        return rotacionDerecha(raiz);
    }

    if (balance < -1 && obtenerBalance(raiz->derecho) <= 0)
        return rotacionIzquierda(raiz);

    if (balance < -1 && obtenerBalance(raiz->derecho) > 0) {
        raiz->derecho = rotacionDerecha(raiz->derecho);
        return rotacionIzquierda(raiz);
    }

    return raiz;
}

Nodo* buscarPalabra(Nodo* nodo, const string& palabra_es) {
    if (nodo == nullptr || nodo->palabra_es == palabra_es)
        return nodo;

    if (palabra_es < nodo->palabra_es)
        return buscarPalabra(nodo->izquierdo, palabra_es);

    return buscarPalabra(nodo->derecho, palabra_es);
}

void cargarPalabrasDesdeArchivo(Nodo*& raiz, const string& archivo_palabras) {
    ifstream archivo(archivo_palabras);
    string es, it, fr, de, en;
    while (archivo >> es >> it >> fr >> de >> en) {
        raiz = insertarNodo(raiz, es, it, fr, de, en);
    }
}

void guardarArbolEnArchivo(Nodo* nodo, ofstream& archivo) {
    if (nodo == nullptr)
        return;

    archivo << nodo->palabra_es << " " << nodo->traducciones["italiano"] << " " 
            << nodo->traducciones["frances"] << " " << nodo->traducciones["aleman"] << " "
            << nodo->traducciones["ingles"] << endl;

    guardarArbolEnArchivo(nodo->izquierdo, archivo);
    guardarArbolEnArchivo(nodo->derecho, archivo);
}

void actualizarArchivoPalabras(Nodo* raiz, const string& archivo_palabras) {
    ofstream archivo(archivo_palabras, ios::trunc);
    guardarArbolEnArchivo(raiz, archivo);
    archivo.close();
}

void reproducirVoz(const string& palabra) {
    char comando[256];
    strcpy(comando, "espeak -v es \"");
    strcat(comando, palabra.c_str());
    strcat(comando, "\"");

    system(comando);
}

void mostrarTraducciones(Nodo* nodo) {
    cout << "Traducciones:" << endl;
    for (const auto& traduccion : nodo->traducciones) {
        cout << traduccion.first << ": " << traduccion.second << endl;
        reproducirVoz(traduccion.second);
    }
}

std::string encriptarPalabra(const std::string& palabra) {
    std::string palabraEncriptada;
    for (char c : palabra) {
        switch (tolower(c)) {
   case 'a':
                palabraEncriptada += "U1";
                break;
            case 'e':
                palabraEncriptada += "U2";
                break;
            case 'i':
                palabraEncriptada += "U3";
                break;
            case 'o':
                palabraEncriptada += "U4";
                break;
            case 'u':
                palabraEncriptada += "U5";
                break;
            case 'b':
            	palabraEncriptada += "m1";
				break;
			case 'c':
				palabraEncriptada += "m2";
				break;
			case 'd':
				palabraEncriptada += "m3";
				break;
			case 'f':
				palabraEncriptada += "m4";
				break;
			case 'g':
				palabraEncriptada += "m5";
				break;
			case 'h':
				palabraEncriptada += "m6";
				break;
			case 'j':
				palabraEncriptada += "m7";
				break;
			case 'k':
				palabraEncriptada += "m8";
				break;
			case 'l':
				palabraEncriptada += "m9";
				break;
			case 'm':
				palabraEncriptada += "n1";
				break;
			case 'n':
				palabraEncriptada += "n2";
				break;
			case 'p':
				palabraEncriptada += "n3";
				break;
			case 'q':
				palabraEncriptada += "n4";
				break;
			case 'r':
				palabraEncriptada += "n5";
				break;
			case 's':
				palabraEncriptada += "n6";
				break;
			case 't':
				palabraEncriptada += "n7";
				break;
			case 'v':
				palabraEncriptada += "n8";
				break;
			case 'w':
				palabraEncriptada += "n9";
				break;
			case 'x':
				palabraEncriptada += "p1";
				break;
			case 'y':
				palabraEncriptada += "p2";
				break;
			case 'z':
				palabraEncriptada += "p3";
				break;
        }
    }
    return palabraEncriptada;
}

void registrarUsuario() {
    string usuario, contrasena;

    cout << "Ingrese el nombre de usuario: ";
    cin >> usuario;
    cout << "Ingrese la contrasena: ";
    cin >> contrasena;

    ofstream archivo("usuarios.txt", ios::app);
    if (archivo.is_open()) {
        archivo << usuario << " " << contrasena << endl;
        archivo.close();

        // Crear carpeta para el usuario
        string carpetaUsuario = "C:\\Users\\Usuario\\Desktop\\Audio\\Audio\\" + usuario;
        _mkdir(carpetaUsuario.c_str());

        cout << "Usuario registrado exitosamente." << endl;
    } else {
        cerr << "No se pudo abrir el archivo de usuarios." << endl;
    }
}

bool iniciarSesion() {
    string usuario, contrasena;
    string u, c;

    cout << "Ingrese el nombre de usuario: ";
    cin >> usuario;
    cout << "Ingrese la contrasena: ";
    cin >> contrasena;

    ifstream archivo("usuarios.txt");
    if (archivo.is_open()) {
        while (archivo >> u >> c) {
            if (u == usuario && c == contrasena) {
                usuarioActual = usuario;  // Guardar el usuario actual
                cout << "Inicio de sesion exitoso." << endl;
                return true;
            }
        }
        archivo.close();
    } else {
        cerr << "No se pudo abrir el archivo de usuarios." << endl;
    }

    cout << "Nombre de usuario o contrasena incorrectos." << endl;
    return false;
}

void guardarHistorial() {
    if (usuarioActual.empty()) return;

    string carpetaUsuario = "C:\\Users\\GAMING PC\\Desktop\\Audio\\" + usuarioActual;
    ofstream archivoOriginal(carpetaUsuario + "\\historial_original.txt", ios::app);
    ofstream archivoEncriptado(carpetaUsuario + "\\historial_encriptado.txt", ios::app);

    for (const string& palabra : historial) {
        archivoOriginal << palabra << endl;
    }
    for (const string& palabraEncriptada : historial_encriptado) {
        archivoEncriptado << palabraEncriptada << endl;
    }

    archivoOriginal.close();
    archivoEncriptado.close();
}

void mostrarHistorial() {
    cout << "---- Historial de Palabras ----" << endl;
    for (size_t i = 0; i < historial.size(); ++i) {
        cout << "Palabra: " << historial[i] << " - Encriptada: " << historial_encriptado[i] << endl;
    }
}

int main() {
    Nodo* raiz = nullptr;
    string archivo_palabras = "palabras.txt";

    cout << "---- Sistema de gestion de palabras ----" << endl;
    cout << "1. Iniciar sesion" << endl;
    cout << "2. Registrar usuario" << endl;
    cout << "Ingrese una opcion: ";
    int opcion;
    cin >> opcion;

    if (opcion == 2) {
        registrarUsuario();
    }

    if (!iniciarSesion()) {
        return 0;
    }

    cargarPalabrasDesdeArchivo(raiz, archivo_palabras);

    while (true) {
        cout << "\n---- Menu ----" << endl;
        cout << "1. Insertar palabra" << endl;
        cout << "2. Eliminar palabra" << endl;
        cout << "3. Buscar palabra" << endl;
        cout << "4. Mostrar historial" << endl;
        cout << "5. Salir" << endl;
        cout << "Ingrese una opcion: ";
        cin >> opcion;

        if (opcion == 1) {
            string es, it, fr, de, en;
            cout << "Ingrese la palabra en español: ";
            cin >> es;
            cout << "Ingrese la traduccion en italiano: ";
            cin >> it;
            cout << "Ingrese la traduccion en frances: ";
            cin >> fr;
            cout << "Ingrese la traduccion en aleman: ";
            cin >> de;
            cout << "Ingrese la traduccion en ingles: ";
            cin >> en;

            raiz = insertarNodo(raiz, es, it, fr, de, en);
            actualizarArchivoPalabras(raiz, archivo_palabras);

            string encriptada = encriptarPalabra(es);
            historial.push_back(es);
            historial_encriptado.push_back(encriptada);
            contadorPalabras[es]++;

            // Guardar historial del usuario
            guardarHistorial();
        } else if (opcion == 2) {
            string palabra;
            cout << "Ingrese la palabra a eliminar (en espanol): ";
            cin >> palabra;
            raiz = eliminarNodo(raiz, palabra);
            actualizarArchivoPalabras(raiz, archivo_palabras);
        } else if (opcion == 3) {
            string palabra;
            cout << "Ingrese la palabra a buscar (en espanol): ";
            cin >> palabra;
            Nodo* resultado = buscarPalabra(raiz, palabra);
            if (resultado != nullptr) {
                mostrarTraducciones(resultado);
                string encriptada = encriptarPalabra(palabra);
                historial.push_back(palabra);
                historial_encriptado.push_back(encriptada);
                contadorPalabras[palabra]++;

                // Guardar historial del usuario
                guardarHistorial();
            } else {
                cout << "Palabra no encontrada." << endl;
            }
        } else if (opcion == 4) {
            mostrarHistorial();
        } else if (opcion == 5) {
            break;
        } else {
            cout << "Opcion invalida." << endl;
        }
    }

    return 0;
}
