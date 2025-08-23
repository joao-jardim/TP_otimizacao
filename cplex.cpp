#include<iostream>
#include<stdio.h>
#include<sstream>

#include <ilcplex/ilocplex.h>

//AINDA NAO ENTENDO BEM ESSES TYPEDEFS...
//nao coloquei nomes nos talhoes. 
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
    printf("Quantidade de Lotes (contando): \t\t %d\n", L);
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

    //L[0] = fabrica. L[L+1] = fabrica como nó final.

    //Declaração do ambiente e do modelo matemático
    IloEnv env;
	IloModel modelo(env);

    //Declaração das variáveis de decisão
   
    //x = Variavel binaria, indica se o veıculo k atende o lote j logo apos i
    //x[veiculo][lotei][lotej]
    IloNumVar3Matrix x(env, V);
    for (int v = 0; v < V; ++v) {
        x[v] = IloNumVarMatrix(env, L+2);
        for (int i = 0; i < L+2; ++i) {
            x[v][i] = IloNumVarArray(env, L+2, 0, 1, ILOBOOL);
            
            for (int j = 0; j < L+2; ++j) modelo.add(x[v][i][j]);
        } 
    }

    //s = Variavel binaria, indica se o veıculo k atende o lote i
    //s[veiculo][lote]
    IloNumVarMatrix s(env, V);
    for (int v = 0; v < V; ++v) {
        s[v] = IloNumVarArray(env, L+2, 0, 1, ILOBOOL);
        for (int i = 0; i < L+2; ++i) modelo.add(s[v][i]);
    }

    //b = Instante em que o veıculo k chega no lote i
    //b[veiculo][lote]
    IloNumVarMatrix b(env, V);
    for (int v = 0; v < V; ++v) {
        b[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(b[v][i]);
    }
    //d = Instante que o veıculo k começa a ser carregado com o lote i
    //d[veiculo][lote]
    IloNumVarMatrix d(env, V);
    for (int v = 0; v < V; ++v) {
        d[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(d[v][i]);
    }
    
    //w = Tempo de espera do veıculo k no talhao para ser carregado com o lote i
    //w[veiculo][lote]
    IloNumVarMatrix w(env, V);
    for (int v = 0; v < V; ++v) {
        w[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(w[v][i]);
    }    
    
    //h = Instante que o lote i foi atendido
    //h[lote]
    IloNumVarArray h(env, L, 0, IloInfinity, ILOFLOAT);
    for (int i = 0; i < L; ++i) modelo.add(h[i]);

    //y = Variavel binaria, indica se a empilhadeira e atende o talhao b logo apos a
    //y[empilhadeira][talhao_a][talhao_b]
    IloNumVar3Matrix y(env, E);
    for (int e = 0; e < E; ++e) {
        y[e] = IloNumVarMatrix(env, T+2);
        for (int a = 0; a < T+2; ++a) {
            y[e][a] = IloNumVarArray(env, T+2, 0, 1, ILOBOOL);
            for (int b = 0; b < T+2; ++b) modelo.add(y[e][a][b]);
        }
    }

    //z = Variavel binaria, indica se a empilhadeira e atende o talhao a
    //z[empilhadeira][talhao]
    IloNumVarMatrix z(env, E);
    for (int e = 0; e < E; ++e) {
        z[e] = IloNumVarArray(env, T, 0, 1, ILOBOOL);
        for (int a = 0; a < T; ++a) modelo.add(z[e][a]);
    }
    
    //c = Instante em que a empilhadeira e começa a atender o talhao a
    //c[empilhadeira][talhao]
    IloNumVarMatrix c(env, E);
    for (int e = 0; e < E; ++e) {
        c[e] = IloNumVarArray(env, T, 0, IloInfinity, ILOFLOAT);
        for (int a = 0; a < T; ++a) modelo.add(c[e][a]);
    }
    
    //m = Instante de inıcio de atendimento do ultimo lote, em horas
    IloNumVar m(env, 0, IloInfinity, ILOFLOAT);
    modelo.add(m);
    
    //Declaração da função objetivo
    IloExpr fo(env);

    fo = m;
    modelo.add(IloMinimize(env, fo));

    //Restrições
    //Expressão 2: Garante que o ultimo lote só podera ser atendido apos todos os anteriores
    for (int i = 0; i < L-1; ++i) {
        IloExpr expr(env);
        expr = h[i] - m;
        
        IloRange rest1(env, -IloInfinity, expr, 0); // -∞ ≤ h[i] - m ≤ 0
        modelo.add(rest1);
        expr.end();
    }
    
    //Expressao 3: Indica que cada lote deve ser atendido por exatamente um veiculo.
    for (int i = 0; i < L; ++i) {
        IloExpr expr(env);
        for (int v = 0; v < V; ++v) {
            expr += s[v][i];
        }
        
        IloRange rest2(env, 1, expr, 1); // 1 ≤ ∑s[v][i] ≤ 1
        modelo.add(rest2);
        expr.end();
    }

    //Expressao 4: assegura que se o veıculo k nao atende o lote i, nao pode atender nenhum outro lote imediatamente antes ou apos.
    for (int k = 0; k < V; ++k) {
        for (int i = 1; i < L; ++i) {
            IloExpr soma_arcos(env);
            for (int j = 1; j <= L; ++j) {
                if (i != j) {
                    soma_arcos += x[k][i][j]; // Soma arcos que saem de i para j
                    soma_arcos += x[k][j][i]; // Soma arcos que chegam em i vindo de j
                }
            }
            modelo.add(soma_arcos == 2 * s[k][i]);
            soma_arcos.end();
        }
    }

    //Expressao 5: assegura que se o veıculo k atende o lote i, deve obrigatoriamente atender algum outro lote imediatamente antes
    //ou apos, mesmo que virtual (representado a garagem).
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) {
            IloExpr expr(env);
            for (int j = 1; j <= L+1; ++j) {
                if (j != i) expr += x[k][i][j];
            }

            for (int j = 0; j <= L; ++j) {
                if (j != i) expr += x[k][j][i];
            }
            
            // se s[k][i] = 1, então só um dos caminhos pode ser verdade
            modelo.add(expr == s[k][i]); 
            expr.end();
        }
    }

    //Expressao 6: garante que o numero de transiçoes realizadas por um veıculo é exatamente uma unidade 
    // menor do que o numero total de lotes atendidos por esse veıculo. Essa restricão é necessária para 
    // evitar a formação de sub-rotas no problema.
    for (int k = 0; k < V; ++k) {
        // Lado Esquerdo: Soma de todas as transições ENTRE LOTES
        IloExpr transicoes(env);
        for (int i = 1; i <= L; ++i) {      // i percorre todos os lotes
            for (int j = 1; j <= L; ++j) {  // j percorre todos os lotes
                if (i != j) {
                    transicoes += x[k][i][j];
                }
            }
        }

        // Lado Direito: Soma de todos os lotes atendidos pelo veículo k
        IloExpr lotes_servidos(env);
        for (int i = 1; i <= L; ++i) {      // i percorre todos os lotes
            lotes_servidos += s[k][i];
        }

        // A restrição só se aplica se o veículo atender pelo menos um lote.
        // Isso evita que o modelo tente satisfazer 0 == -1 para veículos não utilizados.
        modelo.add(IloIfThen(env, lotes_servidos >= 1, transicoes == lotes_servidos - 1));

        transicoes.end();
        lotes_servidos.end();
    }

    //Expressao 7: certifica que todos os veículos irao sair da fabrica 
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int j = 1; j < L+1; ++j) {
            expr += x[k][0][j]; // Considerando o lote 0 como a garagem
        }
        IloRange rest7(env, 1, expr, 1); 
        modelo.add(rest7);
        expr.end();
    }

    // certifica que todos os veículos retornem a fábrica 
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int i = 0; i < L; ++i) {
            expr += x[k][i][L+1]; // Considerando o lote 0 como a garagem
        }
        IloRange rest8(env, 1, expr, 1);
        modelo.add(rest8);
        expr.end();
    }

    //Expressao 9: garante que a quantidade de veículos que sai de um determinado lote é 
    //igual à quantidade que chega no próximo a ser atendido (continuidade de fluxo).
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) {
            IloExpr expr1(env);
            IloExpr expr2(env);

            for (int j = 0; j < L; j++) {  // corrigido
                expr1 += x[k][j][i];
            }
            for (int j = 1; j <= L; j++) {  // corrigido
                expr2 += x[k][i][j];
            }
            IloExpr expr(env);
            expr = expr1 - expr2;
            modelo.add(IloRange(env, 0, expr, 0));
            expr.end();
        }
    }

    //expressão 10 do artigo
    double M = 1e5;
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) {
            // b[k][i] <= Ti[i] + M*(1 - x[k][i][0])
            modelo.add(b[k][i] <= Ti[i] + M * (1 - x[k][i][0]));

            // b[k][i] >= Ti[i] - M*(1 - x[k][i][0])
            modelo.add(b[k][i] >= Ti[i] - M * (1 - x[k][i][0]));
        }
    }

    // //expressão 11 do artigo
    // for (int k = 0; k < V; ++k) {
    //     for (int i = 0; i < L; ++i) {
    //         for (int j = 0; j < L; ++j) {
    //             if (i != j) {
    //                 // Cria a restrição:
    //                 // b[j][k] - b[i][k] - w[i][k] - C - Tv[i] - Ti[j] + M*(1 - x[i][j][k]) >= 0
    //                 modelo.add(
    //                     b[j][k] - b[i][k] - w[i][k] - C - Tv[i] - Ti[j] + M * (1 - x[i][j][k]) >= 0
    //                 );
    //             }
    //         }
    //     }
    // }


    //ERRADO
    // //expressão 12 do artigo
    // for (int k = 0; k < V; ++k) {
    //     for (int i = 0; i < L; ++i) {
    //         IloExpr expr(env);
    //         expr = d[i][k] - b[i][k] - w[i][k];
    //         modelo.add(IloRange(env, 0, expr, 0));
    //         expr.end();
    //     }
    // }
    
    // //expressão 13 do artigo
    // for (int k = 0; k < V; ++k) {
    //     for (int i = 0; i < L; ++i) {
    //         IloExpr expr(env);
    //         expr = d[i][k] - M * s[i][k];
    //         modelo.add(IloRange(env, -IloInfinity, expr, 0));
    //         expr.end();
    //     }
    // }

    // //expressão 14 do artigo
    // for (int k = 0; k < V; ++k) {
    //     for (int i = 0; i < L; ++i) {
    //         IloExpr expr1(env), expr2(env);
    //         expr1 = d[i][k] - h[i] - M * (1 - s[i][k]);
    //         expr2 = d[i][k] - h[i] + M * (1 - s[i][k]);
    //         modelo.add(IloRange(env, -IloInfinity, expr1, 0));
    //         modelo.add(IloRange(env, 0, expr2, IloInfinity));
    //         expr1.end();
    //         expr2.end();
    //     }
    // }

    // //expressão 15 do artigo
    // for (int i = 0; i < L; ++i) {
    //     for (int j = 0; j < L; ++j) {
    //         if (i != j) {
    //             for (int a = 0; a < T; ++a) {
    //                 if (LE[a][i] == 1 && LE[a][j] == 1) {
    //                     IloExpr expr1(env), expr2(env);
    //                     expr1 = h[i] - h[j] - C + M * y[i][j][a];
    //                     expr2 = h[j] - h[i] - C + M * (1 - y[i][j][a]);
    //                     modelo.add(IloRange(env, 0, expr1, IloInfinity));
    //                     modelo.add(IloRange(env, 0, expr2, IloInfinity));
    //                     expr1.end();
    //                     expr2.end();
    //                 }
    //             }
    //         }
    //     }
    // }

    // //expressão 16 do artigo
    // for (int a = 0; a < T; ++a) {
    //     IloExpr expr(env);
    //     for (int e = 0; e < E; ++e) {
    //         expr += z[e][a];
    //     }
    //     modelo.add(IloRange(env, 1, expr, 1));
    //     expr.end();
    // }

    // //expressão 17 do artigo
    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a < T; ++a) {
    //         for (int b = 0; b < T; ++b) {
    //             if (a != b) {
    //                 IloExpr expr1(env), expr2(env);
    //                 expr1 = y[a][b][e] - z[e][a];
    //                 expr2 = y[b][a][e] - z[e][a];
    //                 modelo.add(IloRange(env, -IloInfinity, expr1, 0));
    //                 modelo.add(IloRange(env, -IloInfinity, expr2, 0));
    //                 expr1.end();
    //                 expr2.end();
    //             }
    //         }
    //     }
    // }

    // //expressão 18 do artigo
    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a < T; ++a) {
    //         IloExpr out(env), in(env);
    //         for (int b = 0; b < T; ++b) {
    //             if (a != b) {
    //                 out += y[a][b][e];
    //                 in  += y[b][a][e];
    //             }
    //         }
    //         // out <= z[e][a]
    //         modelo.add(IloRange(env, -IloInfinity, out - z[e][a], 0));

    //         // in <= z[e][a]
    //         modelo.add(IloRange(env, -IloInfinity, in - z[e][a], 0));

    //         // out + in >= z[e][a]
    //         modelo.add(IloRange(env, 0, out + in - z[e][a], IloInfinity));

    //         out.end();
    //         in.end();
    //     }
    // }


    // //expressão 19 do artigo
    // for (int e = 0; e < E; ++e) {
    //     IloExpr expr(env);
    //     for (int b = 1; b <= T+1; ++b) {
    //         expr += y[0][b][e];
    //     }
    //     modelo.add(IloRange(env, 1, expr, 1));
    //     expr.end();
    // }

    // //expressão 20 do artigo 
    // for (int e = 0; e < E; ++e) {
    //     IloExpr expr(env);
    //     for (int a = 0; a <= T; ++a) {
    //         expr += y[a][T+1][e];
    //     }
    //     modelo.add(IloRange(env, 1, expr, 1));
    //     expr.end();
    // }

    // //expressão 21 do artigo 
    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a < T; ++a) {
    //         IloExpr expr(env);
    //         for (int b = 0; b <= T; ++b) {
    //             if (a != b) expr += y[b][a][e];
    //         }
    //         for (int b = 1; b <= T+1; ++b) {
    //             if (a != b) expr -= y[a][b][e];
    //         }
    //         modelo.add(IloRange(env, 0, expr, 0));
    //         expr.end();
    //     }
    // }

    // //expressão 22 do artigo
    // M = 1000000; // valor suficientemente grande

    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a < T; ++a) {
    //         for (int b = 0; b < T; ++b) {
    //             if (a != b) {
    //                 IloExpr expr(env);
    //                 expr = c[e][b] - c[e][a] 
    //                     - (C * LT[a]) - DE[a][b] 
    //                     + M * (1 - y[a][b][e]);
    //                 modelo.add(IloRange(env, 0, expr, IloInfinity));
    //                 expr.end();
    //             }
    //         }
    //     }
    // }

    // //expressão 23 do artigo
    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a < T; ++a) {
    //         IloExpr expr(env);
    //         expr = c[e][a] - M * z[e][a];
    //         modelo.add(IloRange(env, -IloInfinity, expr, 0));
    //         expr.end();
    //     }
    // }

    // //expressão 24 do artigo
    // for (int e = 0; e < E; ++e) {
    //     for (int a = 0; a <= T; ++a) {
    //         for (int b = 0; b <= T; ++b) {
    //             if (a != b) {
    //                 for (int i = 0; i < L; ++i) {
    //                     if (LE[a][i] == 1) {
    //                         // h_i >= c_a^e - M*(1 - y[a][b][e])
    //                         IloExpr expr1(env);
    //                         expr1 = h[i] - c[e][a] + M * (1 - y[a][b][e]);
    //                         modelo.add(IloRange(env, 0, expr1, IloInfinity));
    //                         expr1.end();

    //                         // h_i <= c_b^e + DE[a][b] + M*(1 - y[a][b][e])
    //                         IloExpr expr2(env);
    //                         expr2 = h[i] - c[e][b] - DE[a][b] - M * (1 - y[a][b][e]);
    //                         modelo.add(IloRange(env, -IloInfinity, expr2, 0));
    //                         expr2.end();
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    //Resolução do matemático
    IloCplex cplex(modelo);
    cplex.exportModel("entrada_modelo.lp");
    cplex.solve();

    //Impressão dos resultados
    IloAlgorithm::Status status = cplex.getStatus();
    const char* statusString;
    switch (status) {
        case IloAlgorithm::Optimal:
            statusString = "Optimal";
            break;
        case IloAlgorithm::Infeasible:
            statusString = "Infeasible";
            break;
        case IloAlgorithm::Unbounded:
            statusString = "Unbounded";
            break;
        case IloAlgorithm::Feasible:
            statusString = "Feasible";
            break;
        default:
            statusString = "Unknown";
            break;
    }

    printf("\nStatus da solução: %s\n", statusString);
    printf("\nCusto total: %.2f\n", cplex.getObjValue());
    printf("\nSolução:\n");
    double valor = cplex.getValue(m);
    return 0;
}