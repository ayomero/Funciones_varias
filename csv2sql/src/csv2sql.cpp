//============================================================================
// Name        : csv2sql.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Lee un archivo csv y lo convierte a un sql con INSERT
// Params      : Archivo origen, Directorio destino, Tabla destino, Lineas por archivo
//============================================================================

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

using namespace std;

vector<string> valores;
string primera;   //La primera linea de nombres de campo
vector<int> errores;
int campos;

void SeparaPath(string origen, string &dir, string &arch);
void ArmaValores(vector<string> &tokens, int posicion);
//void ReadFile(string elOrigen, string elDestino, string nArchivo, int tamanho);
void GuardaSQL(string destino,string archivo, string tabla);
// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

//Cambia la coma por el punto
static inline void cambiar(char cara, string& s){
	for(size_t j=0;j<s.length();j++)
		if(s[j]==cara){
			s[j]='.';
		}
}

// Lee el archivo origen para convertir las lineas csv a sql
void ReadFile(string elOrigen, string elDestino, string nArchivo, string tabla, int tamanho = 150){
	string linea;
	vector<string> lineas;
	vector<string> v;
	int archivos=0;   //cuantos archivos van
	int j=0;          //conteo de lineas en el origen
	int k=0;          //conteo de 0 a tamanho
	if(valores.size()>0)
		valores.clear();
	ifstream myfile(elOrigen);
	if(myfile.is_open()){
		while(getline(myfile,linea)){
			cambiar(',',linea);
			stringstream miLinea(linea);
			while(miLinea.good()){
				string subca;
				getline(miLinea, subca, ';');
			    v.push_back(subca);
			}
			if(j==0) campos=v.size();
			else if(campos != (int)v.size())
				errores.push_back(j);
			ArmaValores(v, j);
			++j;++k;
		    v.clear();
			if(k>=tamanho){
				GuardaSQL(elDestino, nArchivo+to_string(archivos++), tabla);
				valores.clear();
				valores.push_back(primera);
				k=0;
			}
		}
		GuardaSQL(elDestino, nArchivo+to_string(archivos), tabla);
	}
}

string laFecha(string elToken){
	stringstream miFecha(elToken);
	string subca;
	string resulta;
	int i=0;
	while(miFecha.good()){
		string subca;
		getline(miFecha, subca, '/');
		trim(subca);
		switch (i){
			case 0: resulta="-"+subca;
				break;
			case 1: resulta = "-"+subca+resulta;
				break;
			case 2: resulta = subca + resulta;
		}
		++i;
	}
	return resulta;
}

int main(int argc, char* argv[]){
	string directorio;
	string archivo;
	int limite;
	campos=0;
	cout << "Hola al mundo." << endl;
	if(argc>0){
		SeparaPath(argv[1],directorio,archivo);
		if(argc<5)
			ReadFile(argv[1],argv[2],archivo,argv[3]);
		else{
			limite = stoi(argv[4]);
			ReadFile(argv[1],argv[2],archivo,argv[3],limite);
		}
	}
	if(errores.size()>0){
		for(int &renglon : errores)
			cout << renglon << ", ";
		cout << "\n";
	}
	return 0;
}

// funcion que separa del origen el directorio y el nombre del archivo
void SeparaPath(string origen, string &dir, string &arch){
    filesystem::path path((std::filesystem::__cxx11::path)origen);
    dir = path.parent_path().string();
    arch = path.filename().string();
}

//funcion que prepara la lista de los valores a insertar en formato sql
void ArmaValores(vector<string> &tokens, int posicion){
	string laLinea;
	string token;
	laLinea = "(";
    for (size_t i = 0; i < tokens.size(); i++){
    	if(i>0) laLinea += ",";
    	if(tokens[i]==" ") tokens[i]="";
    	//if((i==5 or i==9) && posicion>0)   //Para modificar el formato de fecha en bautismo
    	if((i==1) && posicion>0)   //Para modificar el formato de fecha en movimientos
    		tokens[i]=laFecha(tokens[i]);
    	//if(i!=0 && i!=1 && i!=22){  //los campos que no son numericos en bautismo
    	if(i!=2 && i!=5){  //los campos que no son numericos en movimientos
    		token=tokens[i];
    		trim(token);
    		if(posicion>0)
    			laLinea += "'" + token + "'";
    		else
    			laLinea += token;
    	}
    	else{
    		token=tokens[i];
    		trim(token);
    		if(token.length()<=0)
    			token="0";
    		laLinea += token;
    	}
    }
    laLinea += ")";
    if(posicion > 0)
    	laLinea += ",";
    valores.push_back(laLinea);
    if(posicion==0)
    	primera = laLinea;
	//cout << laLinea << endl;
}

void GuardaSQL(string destino,string archivo, string tabla){
	vector<string>::iterator it;
	  it = valores.begin();
	  *it = "INSERT INTO "+ tabla + *it + " VALUES ";
	ofstream output_file(destino+"\\"+archivo+".sql");
	ostream_iterator<string> output_iterator(output_file, "\n");
	copy(begin(valores), end(valores), output_iterator);
	output_file.close();
}
