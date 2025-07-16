#include<iostream>
#include<stdio.h>
#include<sstream>

#include <ilcplex/ilocplex.h>

//AINDA NAO ENTENDO BEM ESSES TYPEDEFS...
//nao coloquei nomes nos talhoes. Sei que nao é necessario. 
//o quao boa prática é nesse projeto?
typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3Matrix;
typedef IloArray<IloNumVar3Matrix> IloNumVar4Matrix;

using namespace std;

int main(int argc, char *argv[]) { 
    //Declarar os conjuntos
    int L;                  //LOTES
    int V;                  //VEICULOS
    int T;                  //TALHOES
    int E;                  //EMPILHADEIRAS
    float C;                //TEMPO DE CARREGAMENTO

    int* LT;                //Total de lotes em um talhão a
    int** LE;               //Matriz bin. Se o lote i pertence ao talhão a
    float* Ti;              //Tempo de ida da fábrica ao lote i
    float* Tv;              //Tempo de volta do lote i para a fábrica
    float** DE;             //Tempo de deslocamento entre o talhão a e b

    //Leitura do arquivo
    FILE* fp = fopen(argv[1],"r");
    if( fp == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }
    
    printf("\nVerificação dos dados:\n");
    //Alocação dos conjuntos
    fscanf(fp, "%d %d %d %d %f", &L, &V, &T, &E, &C);
    printf("Quantidade de Lotes: \t\t %d\n", L);
    printf("Quantidade de Veículos: \t %d\n", V);
    printf("Quantidade de Talhões: \t\t %d\n", T);
    printf("Quantidade de Empilhadeiras: \t %d\n", E);
    printf("Tempo de carregamento: \t\t %.2f horas\n", C);

    LT = new int[T];
    LE = new int*[L]; for (int i=0; i<L; i++) LE[i] = new int[T];
    Ti = new float[L];
    Tv = new float[L];
    DE = new float*[T]; for (int i=0; i<T; i++) DE[i] = new float[T];

    //LT
    printf("\n\nLT: Total de lotes em cada talhão\n");
    for (int i=0; i<T; i++){
        fscanf(fp, "%d", &LT[i]);
        printf("Talhão %d:\t%d lotes\n", i+1, LT[i]);
    }

    //LE    
    printf("\n\nLE: A qual talhão pertence cada lote\n");
    for (int i=0; i<L; i++){
        for (int j=0; j<T; j++) {
            fscanf(fp, "%d", &LE[i][j]);
            if (LE[i][j]) printf("Lote %d:\t Talhão %d\n", i+1, j+1);
        }
    }

    //Ti
    printf("\n\nTempo de ida da fábrica a cada lote\n");
    for (int i=0; i<L; i++) {
        fscanf(fp, "%f", &Ti[i]);
        printf("Tempo da fábrica ao lote %d:\t %.2f horas\n", i+1, Ti[i]);
    }

    //Tv
    printf("\n\nTempo de volta do lote para a fábrica\n");
    for (int i=0; i<L; i++) {
        fscanf(fp, "%f", &Tv[i]);
        printf("Tempo do lote %d à fábrica:\t %.2f\n", i+1, Tv[i]);
    }

    //DE
    printf("\n\nTempo de deslocamento entre os talhões\n");
    for (int i=0; i<T; i++) {
        for (int j=0; j<T; j++) {  
            fscanf(fp, "%f", &DE[i][j]);
            printf("Tempo entre os talhões %d e %d:\t %.2f\n", i+1, j+1, DE[i][j]);
        }    
    }
    //Declaração do ambiente e do modelo matemático

    //Declaração das variáveis de decisão

    //Declaração da função objetivo

    //Restrições

    //Resolução do matemático

    //Impressão dos resultados

    return 0;
}